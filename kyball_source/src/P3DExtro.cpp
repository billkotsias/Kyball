#include "StdAfx.h"
#include "P3DExtro.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "FSLToOgre.h"

#include "game\GameplayCollection.h"
#include "hid\HIDBoss.h"

#include "P3DScripts.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"
#include "fsl\FSLArray.h"

#include "tween\TSin.h"
#include "tween\TCos.h"
#include "tween\TLinear.h"

#include "gui\WiText.h"
#include "sound\SoundBoss.h"
#include "visual\VisualBoss.h"

namespace P3D {

	const double P3DExtro::CAMERA_ROT_SPEED = 0.02;
	const double P3DExtro::EARTH_ROT_SPEED = -0.08;
	const double P3DExtro::ENERGY_HEIGHT = 15.;
	const double P3DExtro::ENERGY_SCALE_TIME = 10.; /// 10! secs
	const double P3DExtro::ENERGY_SCALE_DELAY = 1.5; /// secs

	bool P3DExtro::ShowSuperEnding = false;

	P3DExtro::P3DExtro() : sampleDist(0), sampleStrength(0), originalImageWeight(1), blurWeight(0), runningAnimals(0), notFinished(true) {
		/// initial compositor values = "disabled"
	}

	P3DExtro::~P3DExtro() {
		deleteTweens();
		delete mapCollection;
		for (currentAnimal = animalCollections.begin(); currentAnimal != animalCollections.end(); ) {
			GameplayCollection* collectionToDelete = currentAnimal->first;
			++currentAnimal;
			delete collectionToDelete;
		}
		MyGUI::LayoutManager::getInstance().unloadLayout(_widgets); /// another way to destroy widgets (not individually)
		SceneBaseCreator::getSingletonPtr()->destroySceneBase();
	}

	void P3DExtro::begin() {

		MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);

		/// - create scene base
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		sceneBase->createSceneBase();
		Ogre::SceneManager* sceneMan = sceneBase->getSceneManager();

		/// - create scene nodes
		Ogre::SceneNode* root = sceneMan->getRootSceneNode();

		/// <camera>
		mapCollection = new GameplayCollection();
		Ogre::Vector3 zero = Ogre::Vector3(0,0,0);
		earthRoot = mapCollection->newSceneNode( root, &zero );
		earthRoot->setScale(2,2,2);
		cameraRot = mapCollection->newSceneNode( root, &zero );
		camera = mapCollection->newSceneNode( cameraRot, &Ogre::Vector3(0,0,Map::RADIUS * 2.4 * 2) );
		cameraWrapper.setNode(camera);

		/// - create objects
		Ogre::ResourceGroupManager::getSingletonPtr()->loadResourceGroup("MAP");
		FANLib::FSLClassDefinition* mapfsl = P3DScripts::p3dMap->getRoot();

		/// <camera> & <front-light>
		Ogre::Camera* ogreCam = sceneMan->createCamera("P3DExtro_Cam");
		camera->attachObject(ogreCam);
		OgreBuilder::setCamera( mapfsl->getClass("cameraExtro"), ogreCam );
		sceneBase->setCamera(ogreCam);

		Ogre::Light* light = (Ogre::Light*)mapCollection->storeMovable(sceneMan->createLight("CamLight"));
		light->setType(Ogre::Light::LT_POINT);
		light->setDiffuseColour(Ogre::ColourValue());
		light->setSpecularColour(Ogre::ColourValue());
		light->setVisible(true);
		light->setCastShadows(true);
		camera->attachObject( light );

		/// <skybox>
		OgreBuilder::setSkybox( mapfsl->getClass("skybox"), ogreCam );

		/// <earth>
		OgreBuilder::createNodes( mapfsl->getArray("nodes"), mapCollection->newSceneNode( earthRoot, &zero ), mapCollection );
		if (P3DExtro::ShowSuperEnding)
		{
			static_cast<Ogre::Entity*>(static_cast<Ogre::SceneNode*>(earthRoot->getChild(0)->getChild("EARTH_MAP"))->getAttachedObject(0))->setMaterial(
				static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingletonPtr()->getByName("MAP_MAP2"))
			);
		}
		earthRoot->setVisible(true);

		// energies playlists
		int delayIndex = 0;
		for (std::map<MapOptions::CosmosID, std::string>::const_reverse_iterator it = MapOptions::cosmosIDToStr.rbegin(); it != MapOptions::cosmosIDToStr.rend(); ++it) {
			/// since there are no functions for setting xz and y separately, we "hack" by adding another SceneNode in the scene graph hierarchy
			Ogre::SceneNode* energyNode = sceneMan->getSceneNode( "energy_" + it->second );
			Ogre::SceneNode* energyNode2 = energyNode->createChildSceneNode();
			energyNode2->attachObject( energyNode->detachObject((unsigned short)0) ); /// move mesh to sub-node

			/// height
			energyNode->setScale(1,0,1);
			registerTween( new TSin<Ogre::SceneNode, Ogre::Vector3>(
				energyNode, Ogre::Vector3(1, ENERGY_HEIGHT, 1), ENERGY_SCALE_TIME, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale, ENERGY_SCALE_DELAY*delayIndex)
			);
			/// perimeter
			energyNode2->setScale(0,1,0);
			registerTween( new TLinear<Ogre::SceneNode, Ogre::Vector3>(
				energyNode2, Ogre::Vector3::UNIT_SCALE, ENERGY_SCALE_TIME*2, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale, ENERGY_SCALE_DELAY*delayIndex)
			);
			++delayIndex;
		}

		// SUPER-FX
		/// <NOTE> : Compositors are never destroyed directly! The viewport only gets destroyed!
		Ogre::CompositorManager::getSingletonPtr()->addCompositor(sceneBase->getViewport(0), "Bloom")->addListener(this);
		Ogre::CompositorManager::getSingletonPtr()->setCompositorEnabled(sceneBase->getViewport(0), "Bloom", true);

		Ogre::CompositorManager::getSingletonPtr()->addCompositor(sceneBase->getViewport(0), "Radial Blur")->addListener(this);
		Ogre::CompositorManager::getSingletonPtr()->setCompositorEnabled(sceneBase->getViewport(0), "Radial Blur", true);

		/// compositor playlists
		static const double COMPO_INIT_DELAY = 4.;
		static const double COMPO_BIG_TIME = ENERGY_SCALE_TIME + ENERGY_SCALE_DELAY*(delayIndex-1);
		/// - original image
		this->setOriginalWeight(1);
		TPlaylist* const originalImageList = registerTween( new TSin<P3DExtro, double>(
			this, 0., COMPO_BIG_TIME, &P3DExtro::getOriginalWeight, &P3DExtro::setOriginalWeight, COMPO_INIT_DELAY)
		);
		originalImageList->add( new TLinear<P3DExtro, double>(
			this, 2., ENERGY_SCALE_TIME/2, &P3DExtro::getOriginalWeight, &P3DExtro::setOriginalWeight)
		);
		originalImageList->add( new TLinear<P3DExtro, double>(
			this, 1., ENERGY_SCALE_TIME/2, &P3DExtro::getOriginalWeight, &P3DExtro::setOriginalWeight)
		);
		/// - bloom
		this->setBloomWeight(0);
		TPlaylist* const bloomList = registerTween( new TCos<P3DExtro, double>(
			this, 10., COMPO_BIG_TIME, &P3DExtro::getBloomWeight, &P3DExtro::setBloomWeight, COMPO_INIT_DELAY)
		);
		bloomList->add( new TSin<P3DExtro, double>(
			this, 0.66, ENERGY_SCALE_TIME, &P3DExtro::getBloomWeight, &P3DExtro::setBloomWeight)
			);
		bloomList->add( new TLinear<P3DExtro, double>(
			this, 0.25, ENERGY_SCALE_TIME, &P3DExtro::getBloomWeight, &P3DExtro::setBloomWeight)
			);
		/// - radial
		this->setRadialDistance(0.5); /// constant
		this->setRadialStrength(0.);
		TPlaylist* const radialList = registerTween( new TSin<P3DExtro, double>(
			this, 1.5, ENERGY_SCALE_TIME, &P3DExtro::getRadialStrength, &P3DExtro::setRadialStrength, COMPO_INIT_DELAY + COMPO_BIG_TIME)
		);

		// Camera turn
		static const double CAMERA_DELAY = COMPO_INIT_DELAY + COMPO_BIG_TIME + ENERGY_SCALE_TIME + 6.;
		static const Ogre::Vector3 CAMERA_TURN_DEG(-35.5,0.,0.); /// WTF? Wrong Axis!?
		static const double CAMERA_TURN_SECS = 6.;
		registerTween( new TCos<NodeWrapper, Ogre::Vector3>(
			&cameraWrapper, CAMERA_TURN_DEG/2, CAMERA_TURN_SECS/2, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation, CAMERA_DELAY)
		)->add( new TSin<NodeWrapper, Ogre::Vector3>(
			&cameraWrapper, CAMERA_TURN_DEG, CAMERA_TURN_SECS/2, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation)
		)->id = CAMERA_LIST_ID;

		// My-friggin-GUI
		_widgets = MyGUI::LayoutManager::getInstance().loadLayout("credits.xml");
		for (unsigned int i = 0; i < _widgets.size(); ++i) {
			MyGUI::Widget* mWidget = _widgets.at(i);
			mWidget->setAlpha(0);
			widgets.push_back( new WiText((MyGUI::StaticText*)mWidget) );
		}
		currentCredit = -1;
		nextCredit(CAMERA_DELAY + CAMERA_TURN_SECS + 3.);

		// Animals!!! YUPEE!
		const FANLib::FSLInstance* p3dExtro = P3DScripts::getP3DExtro();
		/// build each animal in a different collection
		const FANLib::FSLArray* const nodesfsl = p3dExtro->getRoot()->getArray("nodes");
		for (unsigned int i = 0; i < nodesfsl->getSize(); ++i) {
			GameplayCollection* animalCol = new GameplayCollection();
			Ogre::SceneNode* animalRot = animalCol->newSceneNode( root, &zero );
			animalCollections[ animalCol ] = NodeWrapper(animalRot);

			OgreBuilder::createNodes(nodesfsl->getArray(i), animalRot, animalCol);
			animalCol->pauseVisuals();
			animalRot->setVisible(false);
		}
		currentAnimal = animalCollections.begin();
		oldestUnpausedAnimal = currentAnimal;

		// Musica
		SoundBoss::getSingletonPtr()->newStreamMusic("Oceanus2");

		// start rendering!
		acceptMouseEvents(true);
		acceptKeyEvents(true);
		Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
		ogreRoot->addFrameListener(this);
		ogreRoot->startRendering();
		// rendering ended!
		ogreRoot->removeFrameListener(this);
	}

	void P3DExtro::nextAnimal(double extraDelay) {
		static const Ogre::Vector3 ANIMAL_TURN_DEG(180.,0.,0.); /// WTF? Wrong Axis!?
		static const double ANIMAL_TURN_SECS = 40.*1;

		static const double TIME_FOR_NEXT_ANIMAL = 10.;
		static const int MAX_RUNNING_ANIMALS = 3;

		GameplayCollection* animalCol = currentAnimal->first;
		animalCol->unpauseVisuals();
		NodeWrapper* animalRot = &currentAnimal->second;
		animalRot->setOrientation(Ogre::Vector3(-1.15,0,0));
		animalRot->getNode()->setVisible(true);
		registerTween( new TCos<NodeWrapper, Ogre::Vector3>(
			animalRot, ANIMAL_TURN_DEG, ANIMAL_TURN_SECS, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation, extraDelay)
		)->id = ANIMAL_LIST_ID;

		/// increase <after>
		if (++currentAnimal == animalCollections.end()) currentAnimal = animalCollections.begin();
		if (++runningAnimals < MAX_RUNNING_ANIMALS) nextAnimal(TIME_FOR_NEXT_ANIMAL * runningAnimals);
	}

	void P3DExtro::nextCredit(double extraDelay) {
		static const double CREDIT_STAY = 10.;
		static const double CREDIT_FADE = 0.5;

		FANLib::FSLArray* fslCredits = P3DScripts::p3dVars->getRoot()->getArray("credits");
		/// increase <before>
		if (++currentCredit >= fslCredits->getSize()) return; /// end of credits

		FANLib::FSLArray* fslCredit = fslCredits->getArray(currentCredit);
		for (int i = 0; i < 2; ++i) {
			MyGUI::Widget* const widget = widgets.at(i)->getWidget();
			widget->setCaption( fslCredit->getCString(i) );
			registerTween( new TLinear<MyGUI::Widget, float>(
				widget, 1., CREDIT_FADE, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha, 0.25 * i + extraDelay)
			)->add( new TLinear<MyGUI::Widget, float>(
				widget, 0., CREDIT_FADE, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha, CREDIT_STAY)
			)->id = CREDIT_LIST_ID * i;
		}
	}

	void P3DExtro::playlistFinished(TPlaylist* list) {
		switch (list->id)
		{
			case CREDIT_LIST_ID:
				nextCredit();
				break;

			case CAMERA_LIST_ID:
				nextAnimal(0);
				break;

			case ANIMAL_LIST_ID:
				oldestUnpausedAnimal->first->pauseVisuals();
				oldestUnpausedAnimal->second.getNode()->setVisible(false);
				if (++oldestUnpausedAnimal == animalCollections.end()) oldestUnpausedAnimal = animalCollections.begin();
				--runningAnimals;
				nextAnimal(0);
				break;
		}
	}

	bool P3DExtro::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		HIDBoss::getSingletonPtr()->captureDevices();
		TPlayer::getSingletonPtr()->run(evt.timeSinceLastFrame);
		VisualBoss::getSingletonPtr()->run(evt.timeSinceLastFrame);

		/// camera ever rotating
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->getSkyBoxNode()->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(-CAMERA_ROT_SPEED * evt.timeSinceLastFrame));
		//cameraRot->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(CAMERA_ROT_SPEED * evt.timeSinceLastFrame));

		/// earth ever rotating
		earthRoot->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(EARTH_ROT_SPEED * evt.timeSinceLastFrame));

		/// animals "waving"
		static const double WAVING = 1.5;
		static double wavePos = 0;
		wavePos += 0.75 * evt.timeSinceLastFrame;
		for (AnimalCollections::iterator it = animalCollections.begin(); it != animalCollections.end(); ++it) {
			Ogre::Node* animalNode = it->second.getNode()->getChild(0);
			animalNode->setPosition(0, WAVING * Ogre::Math::Sin(wavePos+(int)animalNode), animalNode->getPosition().z);
		}

		return notFinished;
	}

	void P3DExtro::onKeyEvent() {
		HIDBoss* hidBoss = HIDBoss::getSingletonPtr();
		//if (hidBoss->isKeyDown(OIS::KC_LEFT)) setRadialDistance(sampleDist-0.02);
		//if (hidBoss->isKeyDown(OIS::KC_RIGHT)) setRadialDistance(sampleDist+0.02);
		//if (hidBoss->isKeyDown(OIS::KC_Z)) setRadialStrength(sampleStrength-0.02);
		//if (hidBoss->isKeyDown(OIS::KC_X)) setRadialStrength(sampleStrength+0.02);
		if (hidBoss->isKeyDown(OIS::KC_SPACE) || hidBoss->isKeyDown(OIS::KC_ESCAPE) || hidBoss->isKeyDown(OIS::KC_P)) notFinished = false;
	}

	void P3DExtro::onMouseDown() {
		HIDBoss* hidBoss = HIDBoss::getSingletonPtr();
		if (hidBoss->isButtonDown(HIDBoss::MOUSE_LEFT) || hidBoss->isButtonDown(HIDBoss::MOUSE_RIGHT)) notFinished = false;
	}

	void P3DExtro::setRadialDistance(const double& value) {
		sampleDist = value;
		//std::cout << "sampleDist" << sampleDist << "\n";
		if (!radialParams.isNull()) radialParams->setNamedConstant("sampleDist", value);
	}
	void P3DExtro::setRadialStrength(const double& value) {
		sampleStrength = value;
		//std::cout << "sampleStrength" << sampleStrength << "\n";
		if (!radialParams.isNull()) radialParams->setNamedConstant("sampleStrength", value);
	}
	void P3DExtro::setBloomWeight(const double& value) {
		blurWeight = value;
		//std::cout << "blurWeight" << blurWeight << "\n";
		if (!bloomParams.isNull()) bloomParams->setNamedConstant("BlurWeight", value);
	}
	void P3DExtro::setOriginalWeight(const double& value) {
		originalImageWeight = value;
		//std::cout << "originalImageWeight" << originalImageWeight << "\n";
		if (!bloomParams.isNull()) bloomParams->setNamedConstant("OriginalImageWeight", value);
	}

	// Note to future Bill: make this notification bullshit into a CompositorParamsManager, passing BLOOM_IDs together and waiting for FragmentProgramParameters to be passed back!
	void P3DExtro::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {
		static const Ogre::uint32 BLOOM_PASS_ID = 12345;
		static const Ogre::uint32 RADIAL_PASS_ID = 34567;
		switch (pass_id) {
			case BLOOM_PASS_ID:
				bloomParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				setOriginalWeight(originalImageWeight);
				setBloomWeight(blurWeight);
				break;
			case RADIAL_PASS_ID:
				radialParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				setRadialDistance(sampleDist);
				setRadialStrength(sampleStrength);
				break;
		}
	}

}