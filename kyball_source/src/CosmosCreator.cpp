#include "StdAfx.h"
#include "game\CosmosCreator.h"

#include "P3DScripts.h"
#include "P3DConfig.h"
#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "TextureShadowBuffer.h"
#include "ShadowManager.h"
#include "BurstBoss.h"
#include "visual\VisualBoss.h"

#include "game\GameplayCollection.h"
#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\Camera.h"
#include "game\cannon\Cannon.h"
#include "game\animal\Animal.h"

#include "game\ai\AIPVAnimator.h"
#include "game\form\BallForm.h"
#include "game\form\BobForm.h"
#include "game\form\BallState.h"
#include "game\net\HexNet.h"
#include "game\net\CubeNet.h"

#include "gui\SubScore.h"

#include <fsl\FSLEnum.h>
#include <fsl\FSLArray.h>
#include <fsl\FSLInstance.h>
#include <fsl\FSLClassDefinition.h>

#include "sound\SoundBoss.h"
#include "OgreAddOns.h"
#include "FSLToOgre.h"

// temporaries, but will most probably be needed in the final code too
#include <OgreLight.h>
#include <OgreColourValue.h>
#include <OgreEntity.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreCamera.h>
#include <OgreBillboardSet.h>
#include <OgreFrustum.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

#include "core\Utils.h"
#include <string>

namespace P3D {

	const char* const CosmosCreator::SHADOW_POSTFIX = "_SHADOW";
	const char* const CosmosCreator::SHADOW_TEXTURE = "Shadow_Texture";
	const char* const CosmosCreator::SHADOW_TEX_GROUP = "General";
	const unsigned int CosmosCreator::SHADOW_SIZE = 1024;

	const unsigned int CosmosCreator::BBSET_BALLS_NUM = 800;

	std::vector<CosmosCreator::NetType> CosmosCreator::enumToNetType;

	CosmosCreator::CosmosCreator() : cosmosCollection(0), hdrListener(0), bloomListener(0), sharpenListener(0) {
		buildOnce();
	}

	CosmosCreator::~CosmosCreator() {
		delete cosmosCollection; /// just in case
	}

	void CosmosCreator::buildOnce() {
		static bool built = false;
		if (built) return;
		built = true;

		/// <P3DAnimations.fsl> must have been parsed by this point

		/// - enum -> anim
		FANLib::FSLEnum* enu = P3DScripts::p3dCosmoi->getEnum("Net");
		enumToNetType.resize(enu->getSize(), CosmosCreator::CUBE);
		enumToNetType[enu->getValue("hex")] = CosmosCreator::HEX;
	}

	void CosmosCreator::create(MapOptions options, int numViews)
	{
		mapOptions = options;

		/// - read script...
		FANLib::FSLClass* cosmos = P3DScripts::p3dCosmoi->getRoot()->getClass(mapOptions.getCosmosStr().c_str());
		double cosmosScale = cosmos->getReal("scale");

		/// - create <SceneBase> (was 2nd!)
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		sceneBase->createSceneBase(numViews);		/// single or multi-player?
		sceneManager = sceneBase->getSceneManager();

		/// - setup shadows (new)
		/// <TODO> : read following values from script
		sceneManager->setShadowColour( Ogre::ColourValue(0.66, 0.66, 0.66) );
		sceneManager->setShadowFarDistance( cosmosScale * cosmos->getReal("shadowFar") );
		if (cosmos->getInt("shadowLISPSM") != 0) {
			sceneManager->setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(new Ogre::LiSPSMShadowCameraSetup()));
		} else {
			sceneManager->setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(new Ogre::FocusedShadowCameraSetup()));
		}

		/// - load cosmos <resources> (was 1st!)
		Ogre::ResourceGroupManager::getSingletonPtr()->loadResourceGroup(mapOptions.getCosmosStr());

		/// - ...set sound scale
		SoundBoss::getSingletonPtr()->worldScale = cosmosScale;

		/// - ...create <materials> that accept custom shadows
		FANLib::FSLArray* shadowMats = cosmos->getArray("shadowMaterials");
		for (unsigned int i = 0; i < shadowMats->getSize(); ++i) {
			std::string baseMaterialName = shadowMats->getCString(i); /// name of material to be modified to accept shadows
			std::string shadowMaterialName = baseMaterialName + SHADOW_POSTFIX; /// custom material name
			Ogre::MaterialPtr material;
			material = (Ogre::MaterialPtr)Ogre::MaterialManager::getSingleton().getByName(baseMaterialName);
			material = material->clone(shadowMaterialName);
			customMaterials.push_back(material);
		}

		/// - ...build <world geometry>
		cosmosCollection = new GameplayCollection();
		OgreBuilder::createNodes( cosmos->getArray("nodes"), sceneManager->getRootSceneNode(), cosmosCollection );

		/// compositor stuff - <TODO> : support multiple viewports (just a f***ing for loop)
		if (P3DConfig::gameQuality == GameQuality::SUPA_DUPA)
		{
			Ogre::Viewport* viewport = sceneBase->getViewport(0);
			bloomListener = new BloomListener();
			bloomListener->setBloomWeight( cosmos->getReal("bloomWeight") );
			bloomListener->setOriginalWeight( cosmos->getReal("bloomOrig") );
			bloomListener->notifyViewportSize(viewport->getActualWidth(),viewport->getActualHeight());
			Ogre::CompositorManager::getSingletonPtr()->addCompositor(viewport, "BloomGame")->addListener(bloomListener);
			Ogre::CompositorManager::getSingletonPtr()->setCompositorEnabled(viewport, "BloomGame", true);

			sharpenListener = new SharpenListener();
			sharpenListener->setSharpen( cosmos->getReal("sharpen") );
			Ogre::CompositorManager::getSingletonPtr()->addCompositor(viewport, "Sharpen Edges")->addListener(sharpenListener);
			Ogre::CompositorManager::getSingletonPtr()->setCompositorEnabled(viewport, "Sharpen Edges", true);
			//hdrListener->notifyViewportSize(viewport->getActualWidth(), viewport->getActualHeight());
			//hdrListener->notifyCompositor(hdrCompositor);
		}
	}

	void CosmosCreator::uncreate()
	{
		/// - remove compositors
		Ogre::CompositorManager::getSingletonPtr()->removeCompositorChain(SceneBaseCreator::getSingletonPtr()->getViewport(0));
		delete bloomListener;
		bloomListener = 0;
		delete sharpenListener;
		sharpenListener = 0;

		/// - destroy custom collecion
		delete cosmosCollection;
		cosmosCollection = 0;

		/// - there should be no more collections left; destroy VisualBoss now
		VisualBoss::getSingletonPtr()->deleteAll(); /// the remainings (i.e if bugs lurking...)

		/// - remove custom <materials>
		for (int i = customMaterials.size() - 1; i >= 0; --i) {
			Ogre::MaterialManager::getSingletonPtr()->remove(customMaterials[i]->getName());
		}
		customMaterials.clear();

		/// - destroy decal <frustums>
		for (int i = decalFrustums.size() - 1; i >= 0; --i) {
			delete decalFrustums[i];
		}
		decalFrustums.clear();

		/// - remove <shadow-buffers>
		for (int i = customShadowBuffers.size() - 1; i >= 0; --i) {
			delete customShadowBuffers[i];
		}
		customShadowBuffers.clear();

		/// - remove custom <textures>
		for (int i = customTextures.size() - 1; i >= 0; --i) {
			Ogre::TextureManager::getSingletonPtr()->remove(customTextures[i]);
		}
		customTextures.clear();

		/// - destroy <SceneBase> (inc. <world geometry> & everything else left in Scene)
		SceneBaseCreator::getSingletonPtr()->destroySceneBase();
		sceneManager = 0;

		/// - unload <resources>
		Ogre::ResourceGroupManager::getSingletonPtr()->unloadResourceGroup(mapOptions.getCosmosStr());

		mapOptions.reset();

		/// - remove shadows
		//sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE); // just can't remove with Ogre 1.6
	}

	GameplayBase* CosmosCreator::buildGameplayBase(unsigned int num)
	{
		/// - build <GameplayCollection> for this base (nodes & movables are stored here altogether)
		GameplayCollection* collection = new GameplayCollection();


		/// - build '<GAMEPLAY>' node
		FANLib::FSLClass* cosmos = P3DScripts::p3dCosmoi->getRoot()->getClass(mapOptions.getCosmosStr().c_str());
		FANLib::FSLClass* basefsl = cosmos->getArray("bases")->getClass(num);
		double gameplayScale = cosmos->getReal("scale");
		double gameplayYaw = basefsl->getClass("rotation")->getReal("y"); /// currently only <Y-AXIS> rotation is supported <!>
		Ogre::Vector3 gameplayPosition = FSLToOgre::vector3ToVector3( basefsl->getClass("position") );
		/// - create and attach 'gameplayNode'
		Ogre::SceneNode* gameplayNode;
		{
			Ogre::SceneNode* gameplayParentNode;
			std::string gameplayParentNodeName = basefsl->getCString("nodeContainer");
			if (gameplayParentNodeName.empty()) {
				gameplayParentNode = sceneManager->getRootSceneNode();
			} else {
				gameplayParentNode = sceneManager->getSceneNode(gameplayParentNodeName);
			}
			gameplayNode = collection->newSceneNode(gameplayParentNode, &gameplayPosition);
		}
		gameplayNode->setScale(gameplayScale, gameplayScale, gameplayScale);
		gameplayNode->yaw(Ogre::Degree(gameplayYaw));


		/// - <net> structure
		Ogre::SceneNode* netNode = collection->newSceneNode(gameplayNode, &FSLToOgre::vector3ToVector3(cosmos->getClass("netOff")) );
		Net* net;
		switch ( enumToNetType[ cosmos->getInt("net") ] ) {

			case CosmosCreator::CUBE:
			default:
				net = (CubeNet*)(new CubeNet(BallState::BALL_RADIUS * 2, netNode))->init();
				break;

			case CosmosCreator::HEX:
				net = (HexNet*)(new HexNet(BallState::BALL_RADIUS * 2, netNode))->init();
				break;

		}


		// custom shadow projection
		/// - create <shadow> texture & buffer
		std::string textureName = OgreBuilder::baseName(SHADOW_TEXTURE, num);
		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().createManual(
			textureName, SHADOW_TEX_GROUP,
			Ogre::TEX_TYPE_2D, SHADOW_SIZE, SHADOW_SIZE, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		customTextures.push_back(textureName); /// store to remove later on <uncreate()> : these textures are <SHARED> among all GameplayBases!

		TextureShadowBuffer* shadowBuffer = new TextureShadowBuffer(texPtr->getBuffer(0, 0));
		customShadowBuffers.push_back(shadowBuffer);

		/// - create <Frustum> that projects <shadows>
		Ogre::Frustum* decalFrustum = new Ogre::Frustum();
		decalFrustums.push_back(decalFrustum);
		decalFrustum->setAspectRatio(1);
		decalFrustum->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
		decalFrustum->setOrthoWindowHeight( cosmos->getReal("frustumSize") ); /// the less, the higher shadows resolution!

		Ogre::Vector3 frustumOffset( FSLToOgre::vector3ToVector3( cosmos->getClass("frustumOffset") ) );
		Ogre::SceneNode* frustumNode = collection->newSceneNode(gameplayNode, &frustumOffset);
		frustumNode->attachObject(decalFrustum);
		frustumNode->pitch(Ogre::Degree(-90));
		
		Ogre::Vector2 frustumSize(decalFrustum->getOrthoWindowWidth(), decalFrustum->getOrthoWindowHeight());
		ShadowManager* shadowManager = new ShadowManager(shadowBuffer, &frustumSize, &Ogre::Vector2(-frustumOffset.x, -frustumOffset.z), gameplayScale);

		/// - update all custom materials to accept these shadow projections
		for (int i = customMaterials.size() - 1; i >= 0; --i) {
			Ogre::MaterialPtr material = customMaterials[i];
			if (!material->getBestTechnique()) material->load();
			Ogre::Pass *pass = material->getBestTechnique()->createPass();
			pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			pass->setDepthBias( cosmos->getReal("shadowDepthBias") ); // NOTE : was 0.1!
			pass->setDiffuse(Ogre::ColourValue(0, 0, 0, cosmos->getReal("shadowAlpha") )); /// <alpha> not equal for all ground colours!
			pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128, true); /// <???> Does this really help?
			Ogre::TextureUnitState *texState = pass->createTextureUnitState(textureName);
			texState->setProjectiveTexturing(true, decalFrustum);
			texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
			texState->setTextureBorderColour(Ogre::ColourValue(0, 0, 0, 0));
			texState->setTextureFiltering(Ogre::FO_POINT, Ogre::FO_LINEAR, Ogre::FO_NONE);
		}
		// custom shadow projection


		/// - cube size
		Ogre::Vector3 cubeSize( FSLToOgre::vector3ToVector3(cosmos->getClass("cubeSize")) );


		/// - build <cannon>
		double cannonNodeHeight = basefsl->getReal("cannonHeight");
		Ogre::SceneNode* cannonNode = collection->newSceneNode(gameplayNode, &Ogre::Vector3(0,cannonNodeHeight,-1.1*net->getZDiff()));
		double tubeHeight = cubeSize.y/2. - cannonNodeHeight;
		Ogre::SceneNode* tubeNode = collection->newSceneNode(cannonNode, &Ogre::Vector3(0,tubeHeight,0));
		Cannon* cannon = new Cannon(cannonNode, tubeNode);
		cannon->setTimeBetweenShots(cosmos->getReal("cannonDelay"));


		/// - build <camera>
		Ogre::SceneNode* cameraTarget = collection->newSceneNode(gameplayNode, &Ogre::Vector3(0, cubeSize.y/2., cubeSize.z/2));
		Ogre::SceneNode* cameraNode = collection->newSceneNode(cameraTarget, &Ogre::Vector3::ZERO);

		Ogre::Camera* camera = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createCamera(std::string("P3D::Camera"+num));
		camera->setPosition(0,0,0);
		cameraNode->attachObject(camera);
		camera->setAutoTracking(true, cameraTarget, Ogre::Vector3::ZERO); /// NOTE : makes camera orientation <up-right> ! Should be script-dependent?

		double cameraDistance = cosmos->getReal("cameraDistance");
		double cameraMinHeight = cosmos->getReal("cameraMinHeight");
		OgreBuilder::setCamera( cosmos->getClass("camera"), camera );

		P3D::Camera* gameCamera = new P3D::Camera(cameraNode, cameraTarget, cameraDistance, cameraMinHeight, camera); /// <NOTE> : Camera passed is destroyed by <P3D::Camera> !
		SceneBaseCreator::getSingletonPtr()->setCamera(camera);		

		/// - BurstBoss
		BurstBoss* burst = new BurstBoss(gameplayScale); /// <FUTURE NOTE> : 1 is super-sufficient for ALL bases <!!!>


		/// - build <animal>
		Ogre::SceneNode* animalNode = collection->newSceneNode(gameplayNode, &FSLToOgre::vector3ToVector3(basefsl->getClass("animalPos")));

		/// - <talk>
		{
			Ogre::SceneNode* talkNode = collection->newSceneNode(animalNode, &FSLToOgre::vector3ToVector3(basefsl->getClass("talkPos")));
			double talkScale = basefsl->getReal("talkScale");
			talkNode->setScale( talkScale,talkScale,talkScale );
			Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();
			OgreBuilder::createSphereEntity( OgreBuilder::baseName("talkEntity",num), 16, 16, true, "animal_talk_swarm", talkNode);

			Ogre::ColourValue emissive = FSLToOgre::rgbToColourValue( basefsl->getClass("talkColour") );
			static_cast<Ogre::MaterialPtr>( matMan->getByName("animal_talk_swarm") )->getTechnique(0)->getPass(0)->setSelfIllumination( emissive );
			static_cast<Ogre::MaterialPtr>( matMan->getByName("Hint/EdgesTB") )->getTechnique(0)->getPass(1)->getTextureUnitState(3)->setColourOperationEx(Ogre::LBX_MODULATE,Ogre::LBS_MANUAL,Ogre::LBS_CURRENT, emissive );
			static_cast<Ogre::MaterialPtr>( matMan->getByName("Hint/EdgesLR") )->getTechnique(0)->getPass(1)->getTextureUnitState(3)->setColourOperationEx(Ogre::LBX_MODULATE,Ogre::LBS_MANUAL,Ogre::LBS_CURRENT, emissive );
			static_cast<Ogre::MaterialPtr>( matMan->getByName("Hint/Corners") )->getTechnique(0)->getPass(1)->getTextureUnitState(3)->setColourOperationEx(Ogre::LBX_MODULATE,Ogre::LBS_MANUAL,Ogre::LBS_CURRENT, emissive );
		}

		double animalScale = basefsl->getReal("animalScale");
		if (animalScale < 0)
			animalNode->setScale(animalScale,-animalScale,-animalScale);
		else
			animalNode->setScale(animalScale,animalScale,animalScale);
		animalNode->yaw(Ogre::Degree( basefsl->getClass("animalRot")->getReal("y") )); /// <Y-AXIS> rotation only

		Ogre::Entity* animalEnt = buildCosmosEntity(cosmosName("animal"), animalNode, num);
		collection->storeMovable(animalEnt);
		animalEnt->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_AVERAGE); /// correct skeletal blending for our animals
		Animal* animal = new Animal(animalEnt, mapOptions.getCosmosStr(), burst);


		/// - read from script/calculate <start/end> position of balls <???>
		Ogre::SceneNode* ballStartNode = collection->newSceneNode(cannonNode, &Ogre::Vector3(0,0.5,0));
		Ogre::Vector3 start = OgreAddOns::getDerivedPosition(ballStartNode);
		Ogre::Vector3 end = OgreAddOns::getDerivedPosition(tubeNode);


		/// - AIPVAnimator
		AIPVAnimator* aipv = new AIPVAnimator();


		/// ---> compile 'GameplayBase' structure
		GameplayBase* base = new GameplayBase(num, gameplayNode, mapOptions.getCosmosID(), cannon, gameCamera, net, animal, start, end, collection,
			shadowManager, aipv, burst, cubeSize);


		// initialise components after compilation

		/// - create standard BillboardSets for this base;
		/// - <TODO> : read from script radius, frames, types, max balls
		Ogre::BillboardSet* ballsBBset = base->setBBSet(GameplayBase::BALLS,
			mapOptions.getCosmosStr() + "_balls", BallState::BALL_RADIUS * 2, BallState::BALL_RADIUS * 2, BallForm::BBB_MAX_FRAME, BallState::MAX_TYPES, BBSET_BALLS_NUM, true
			);
		gameplayNode->attachObject(ballsBBset);

		Ogre::BillboardSet* bobsBBset = base->setBBSet(GameplayBase::BOBS,
			"bobs", BallState::BALL_RADIUS * 2, BallState::BALL_RADIUS * 2, BobForm::BOBS_MAX_FRAME, BallState::MAX_BOBS, 20, true
			);
		gameplayNode->attachObject(bobsBBset);

		Ogre::BillboardSet* ballsAlphaBBset = base->setBBSet(GameplayBase::BALLS_ALPHA,
			mapOptions.getCosmosStr() + "_balls_alpha", BallState::BALL_RADIUS * 2, BallState::BALL_RADIUS * 2, BallForm::BBB_MAX_FRAME, BallState::MAX_TYPES, 2, true
			);
		gameplayNode->attachObject(ballsAlphaBBset);

		Ogre::BillboardSet* aimBBset = base->setBBSet(GameplayBase::AIM,
			"aim", BallState::BALL_RADIUS * 0.5, BallState::BALL_RADIUS * 0.5, 1, 1, 30, false
			);
		gameplayNode->attachObject(aimBBset);

		Ogre::BillboardSet* aimHelpBBset = base->setBBSet(GameplayBase::AIM_HELP,
			"aimHelp", BallState::BALL_RADIUS * 2, BallState::BALL_RADIUS * 2, 1, 1, 2, true
			);
		gameplayNode->attachObject(aimHelpBBset);

		Ogre::BillboardSet* ballBoomBBset = base->setBBSet(GameplayBase::BALL_BOOM,
			"ballBoom", BallState::BALL_RADIUS * 3, BallState::BALL_RADIUS * 3, 1, 1, 20, false
			);
		gameplayNode->attachObject(ballBoomBBset);

		/// - Cannon : must be initialized <AFTER> BBSets!
		cannon->init(base);


		/// --- build Cosmos 'GameplayObject's ---
		/// ... IF ANY!!!
		switch (mapOptions.getCosmosID()) {
			case MapOptions::ARCTIC:
			/*case MapOptions::SAHARA: group similar levels together like this */;
				break;
		}


		/// - init & prefabricate "bursts" : depends on <container> (for <deflectors>)
		/// -- bottom
		burst->newDeflector( OgreAddOns::getDerivedPosition(gameplayNode), gameplayNode->_getDerivedOrientation() * Ogre::Vector3(0,1,0), 0.8 ); /// point, normal, bounce
		/// -- sides
		Ogre::SceneNode* helpDef = gameplayNode->createChildSceneNode(); /// <NOTE> : Shouldn't I delete this before 'return' ?!?!?!
		helpDef->setPosition(cubeSize.x/2,0,0);
		burst->newDeflector( OgreAddOns::getDerivedPosition(helpDef), helpDef->_getDerivedOrientation() * Ogre::Vector3(-1,0,0), 0.8 ); /// point, normal, bounce
		helpDef->setPosition(-cubeSize.x/2,0,0);
		burst->newDeflector( OgreAddOns::getDerivedPosition(helpDef), helpDef->_getDerivedOrientation() * Ogre::Vector3(1,0,0), 0.8 ); /// point, normal, bounce

		burst->prefab();

		return base;
	}

	std::string CosmosCreator::cosmosName(const std::string& name) {
		return mapOptions.getCosmosStr() + "_" + name;
	}

	Ogre::Entity* CosmosCreator::buildCosmosEntity(std::string name, Ogre::SceneNode* node, int num) {
		return OgreBuilder::createEntity(OgreBuilder::baseName(name, num), name + ".mesh", false, "", node, true);
	}

	// Compositor stuff
	//---------------------------------------------------------------------------

	void BloomListener::setBloomWeight(const double& value) {
		blurWeight = value;
		if (!bloomParams.isNull()) bloomParams->setNamedConstant("BlurWeight", value);
	}
	void BloomListener::setOriginalWeight(const double& value) {
		originalImageWeight = value;
		if (!bloomParams.isNull()) bloomParams->setNamedConstant("OriginalImageWeight", value);
	}
	BloomListener* BloomListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
		// Calculate gaussian texture offsets & weights
		float deviation = 5.0f;
		float texelSize = 3.0f / (float)std::min(mVpWidth, mVpHeight);

		// central sample, no offset
		mBloomTexOffsetsHorz[0][0] = 0.0f;
		mBloomTexOffsetsHorz[0][1] = 0.0f;
		mBloomTexOffsetsVert[0][0] = 0.0f;
		mBloomTexOffsetsVert[0][1] = 0.0f;
		mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
			mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
		mBloomTexWeights[0][3] = 1.0f;

		// 'pre' samples
		for(int i = 1; i < 8; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
				mBloomTexWeights[i][2] = Ogre::Math::gaussianDistribution(i, 0, deviation);
			mBloomTexWeights[i][3] = 1.0f;
			mBloomTexOffsetsHorz[i][0] = i * texelSize;
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = i * texelSize;
		}
		// 'post' samples
		for(int i = 8; i < 15; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
				mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
			mBloomTexWeights[i][3] = 1.0f;

			mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
		}

		return this;
	}
	void BloomListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {
		static const Ogre::uint32 BLOOM_PASS_ID = 12345;
		static const Ogre::uint32 BLOOM_BLUR_H_PASS_ID = 701;
		static const Ogre::uint32 BLOOM_BLUR_V_PASS_ID = 700;
		switch (pass_id)
		{
			case BLOOM_PASS_ID:
				bloomParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				setOriginalWeight(originalImageWeight);
				setBloomWeight(blurWeight);
				break;

			case BLOOM_BLUR_H_PASS_ID: // blur horz
			{
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
				break;
			}
			case BLOOM_BLUR_V_PASS_ID: // blur vert
			{
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
				break;
			}
		}
	}

	void SharpenListener::setSharpen(const double& value) {
		sharpen = value;
		if (!bloomParams.isNull()) bloomParams->setNamedConstant("Sharpen", value);
	}
	void SharpenListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {
		static const Ogre::uint32 SHARPEN_PASS_ID = 56565;
		switch (pass_id) {
			case SHARPEN_PASS_ID:
				bloomParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				setSharpen(sharpen);
				break;
		}
	}
	//---------------------------------------------------------------------------

	void HDRListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
	}
	void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
	{
		instance->addListener(this);

		// Get some RTT dimensions for later calculations
		Ogre::CompositionTechnique::TextureDefinitionIterator defIter =
			instance->getTechnique()->getTextureDefinitionIterator();
		while (defIter.hasMoreElements())
		{
			Ogre::CompositionTechnique::TextureDefinition* def =
				defIter.getNext();
			if(def->name == "rt_bloom0")
			{
				mBloomSize = (int)def->width; // should be square
				// Calculate gaussian texture offsets & weights
				float deviation = 3.0f;
				float texelSize = 0.5f / (float)mBloomSize;

				// central sample, no offset
				mBloomTexOffsetsHorz[0][0] = 0.0f;
				mBloomTexOffsetsHorz[0][1] = 0.0f;
				mBloomTexOffsetsVert[0][0] = 0.0f;
				mBloomTexOffsetsVert[0][1] = 0.0f;
				mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
					mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
				mBloomTexWeights[0][3] = 1.0f;

				// 'pre' samples
				for(int i = 1; i < 8; ++i)
				{
					mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
						mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution(i, 0, deviation);
					mBloomTexWeights[i][3] = 1.0f;
					mBloomTexOffsetsHorz[i][0] = i * texelSize;
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = i * texelSize;
				}
				// 'post' samples
				for(int i = 8; i < 15; ++i)
				{
					mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
						mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
					mBloomTexWeights[i][3] = 1.0f;

					mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
				}

			}
		}
	}
	void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		//case 994: // rt_lum4
		case 993: // rt_lum3
		case 992: // rt_lum2
		case 991: // rt_lum1
		case 990: // rt_lum0
			break;
		case 800: // rt_brightpass
			break;
		case 701: // rt_bloom1
			{
				// horizontal bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		case 700: // rt_bloom0
			{
				// vertical bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		}
	}
}