#include "StdAfx.h"
#include "map\Map.h"
#include "map\MapOptions.h"
#include "menu\Player.h"
#include "map\SubMap.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "hid\HIDBoss.h"
#include "math\Math.h"
#include "game\GameplayCollection.h"
#include "game\form\BallForm.h"
#include "game\form\BallState.h"
#include "gui\HintBox.h"
#include "gui\MenuBox.h"
#include "gui\SubBlack.h"
#include "FSLToOgre.h"

#include "sound\SoundBoss.h"

#include "P3DWindow.h"
#include "P3DScripts.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"
#include "fsl\FSLArray.h"

#include "tween\TLinear.h"
#include "tween\TSin.h"
#include "tween\TCos.h"

#include <algorithm>
#include <core\Utils.h>

namespace P3D {

	void MapEntity::setScale(const double &s) {
		scale = s;
		if (center) center->setScale(scale, scale, scale);
	}

	MapEntity::~MapEntity() {
		deleteTweens();
	}

	//

	const double Map::ZOOM_TIME = 1.; /// 1. !!!
	const double Map::FACTOR_END = 0.15;
	const double Map::YROT_MAX = 85.;
	const int Map::ZOOM_IN_ID = 1;
	const int Map::ZOOM_OUT_ID = 2;
	const int Map::FADE_IN_ID = 3;
	const int Map::DEMO_OUT_ID = 4;

	//const std::string Map::TXT_ZOOM_OUT = "#afffffMove the mouse and #ffa0a0click#afffff an active area\n#ffa0a0Right-click#afffff to go back";
	//const std::string Map::TXT_ZOOM_IN = "#ffa0a0Click#afffff to enter this area\n#ffa0a0Right-click#afffff to go back";
	//const std::string Map::TXT_ZOOM_OUT = "";
	//const std::string Map::TXT_ZOOM_IN = "";

	const double Map::DAY_TIME = 200.;			/// secs
	const double Map::POLAR_TIME = 2*Pi/350.;	/// radians / secs
	const double Map::FULL_ROT = 360.;			/// degrees
	const double Map::SCALE_TIME = 0.7;			/// degrees
	const double Map::RADIUS = 50.;				/// Earth mesh radius : don't touch! Defined in MAX!

	Map::Map() : mapCollection(0), demoCollection(0), base(0), mapText(0), hintBox(0), subBlack(0) {
	}

	Map::~Map() {
		unload();
	}

	void Map::init() {

		if (mapCollection) return; /// <already> initialized

		/// play menu/map music
		SoundBoss::getSingletonPtr()->freeAllSounds(false);		/// stop and free all other playing sounds
		SoundBoss::getSingletonPtr()->newStreamMusic("menu");

		mapCollection = new GameplayCollection();
		hacked = false;

		/// - create scene base
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		sceneBase->createSceneBase();
		Ogre::SceneManager* sceneMan = sceneBase->getSceneManager();

		/// - create scene nodes
		Ogre::SceneNode* root = sceneMan->getRootSceneNode();

		/// <camera>
		Ogre::Vector3 zero = Ogre::Vector3(0,0,0);
		earthRoot = mapCollection->newSceneNode( root, &zero );
		earthWrapper.setNode(earthRoot);
		cameraRot = mapCollection->newSceneNode( root, &zero );
		cameraRotWrapper.setNode(cameraRot);
		camera = mapCollection->newSceneNode( cameraRot, &Ogre::Vector3(0,0,RADIUS * 2.4) );
		//camera->yaw(Ogre::Degree(180));
		//cameraTarget = mapCollection->newSceneNode( mapRoot, &zero );

		/// <mouse>
		mouseRot = mapCollection->newSceneNode( earthRoot, &zero );
		mouse = mapCollection->newSceneNode( mouseRot, &Ogre::Vector3(0,0,RADIUS) );

		/// <earth>
		earth = mapCollection->newSceneNode( earthRoot, &zero );
		rot.x = rand() * 360. / ((double)RAND_MAX);
		rot.y = (rand() - RAND_MAX / 2.) * YROT_MAX / ((double)RAND_MAX);
		time = asin(rot.y / (YROT_MAX * 2. / 3.)) / POLAR_TIME;

#ifndef _DEPLOY
		FANLib::FSLClassDefinition* p3dLevels = P3DScripts::p3dLevels->getRoot();
		try {
			int cosmos = p3dLevels->getInt("currentCosmos");
			int level = p3dLevels->getInt("currentLevel");
			hacked = true; /// means don't build map (speeds up start-up process)
			return;
		} catch(...) {}
#endif

		/// - create objects
		Ogre::ResourceGroupManager::getSingletonPtr()->loadResourceGroup("MAP");
		FANLib::FSLClassDefinition* mapfsl = P3DScripts::p3dMap->getRoot();

		/// <camera> & <front-light>
		Ogre::Camera* ogreCam = sceneMan->createCamera("Map_Camera");
		mapCollection->storeMovable(ogreCam);
		camera->attachObject(ogreCam);
		OgreBuilder::setCamera( mapfsl->getClass("camera"), ogreCam );
		//ogreCam->lookAt(cameraTarget->_getDerivedPosition());
		//ogreCam->setAutoTracking(true, cameraTarget);

		Ogre::Light* light = (Ogre::Light*)mapCollection->storeMovable(sceneMan->createLight("CamLight"));
		light->setType(Ogre::Light::LT_POINT);
		light->setDiffuseColour(Ogre::ColourValue::White);
		light->setSpecularColour(Ogre::ColourValue::White);
		light->setVisible(true);
		light->setCastShadows(false);
		//light->setAttenuation(RADIUS, 0., 1., 1.);
		camera->attachObject( light );

		/// <skybox>
		OgreBuilder::setSkybox( mapfsl->getClass("skybox"), ogreCam );
		sceneBase->setCamera( ogreCam );

		/// <mouse>
		mBBSet = sceneMan->createBillboardSet("mapPointerBBSet");
		mapCollection->storeMovable( mBBSet );
		mBBSet->setMaterialName("map_pointer");
		mBBSet->setDefaultDimensions(RADIUS / 10., RADIUS / 10.);
		mBBSet->setBillboardRotationType(Ogre::BBR_VERTEX);	/// vertex rotation
		mBBSet->setTextureStacksAndSlices(3, 6);			/// graphics slices
		mBBSet->setUseAccurateFacing(true);					/// essential when <close> to billboards!
		mBBSet->createBillboard(0,0,0)->setTexcoordIndex(0);
		mouse->attachObject( mBBSet );

		/// <earth>
		OgreBuilder::createNodes( mapfsl->getArray("nodes"), earth, mapCollection );

		/// - create map entities
		entities.reserve( MapOptions::cosmosIDToStr.size() + 1 );
		for (std::map<MapOptions::CosmosID, std::string>::const_iterator it = MapOptions::cosmosIDToStr.begin(); it != MapOptions::cosmosIDToStr.end(); ++it) {
			std::string nodeName = "Cam_" + it->second;
			entities.push_back( MapEntity(
				it->first,
				sceneMan->getSceneNode( nodeName ),
				sceneMan->getSceneNode( "energy_" + it->second )
				) );
		}
		selectedEntity = &unselectedEntity;

		/// - misc
		minimumDistance = mapfsl->getReal("minimumDistance");

		/// - further graphics "init"
		//runDemoFrame(0);
		earthRoot->setScale(0,0,0);
		earthRoot->setVisible(false);
		if (!mapText) mapText = new SubMap("map");
		if (!hintBox) {
			hintBox = new HintBox("Overlay/MapInfo");
			hintBox->delay = ZOOM_TIME;
			hintBox->setFinalSize(0.18,0.25);
			hintBox->setFinalFontHeight(0.035);
			hintBox->setRelativePosition(0.93, 0.8);
		}
		if (!subBlack) {
			subBlack = new SubBlack("map");
			subBlack->setCallBack(this, &Map::gameOn);
			subBlack->fadeTime = 0.66;
		}
	}

	void Map::showDemo() {

		static const unsigned int ballsNum = 8;
		static const Ogre::Real ballsDim = 2.;
		static const double minDist = 10.;
		static const int xDiv = 1;
		static const int yDiv = 8;
		static const double fadeInTime = 3.;

		demoCollection = new GameplayCollection();

		/// create stuff
		demoBalls.clear();
		demoBalls.resize(ballsNum);
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();

		/// - balls
		Ogre::BillboardSet* bbset;
		for (unsigned int i = 0; i < ballsNum; ++i) {
			Ogre::SceneNode* node = demoCollection->newSceneNode(camera, &Ogre::Vector3((rand() - RAND_MAX / 2.) * 2 * minDist / RAND_MAX,0,0));	/// rotation node
			Ogre::SceneNode* bbNode = demoCollection->newSceneNode(node, &Ogre::Vector3(0,0, (i + minDist) * ballsDim * 1.1));	/// billboard node
			bbset = (Ogre::BillboardSet*)demoCollection->storeMovable(sceneMan->createBillboardSet(std::string("demoMap") + FANLib::Utils::toString(i), 1));
			bbset->setMaterialName("demoMapBalls");
			bbset->setDefaultDimensions(ballsDim,ballsDim);
			bbset->setBillboardRotationType(Ogre::BBR_VERTEX);									/// vertex rotation
			bbset->setTextureStacksAndSlices(BallState::MAX_TYPES, BallForm::BBB_MAX_FRAME);	/// graphics slices
			Ogre::Billboard* bb = bbset->createBillboard(0,0,0);
			bb->setTexcoordIndex(i);
			bbNode->attachObject(bbset);
			node->roll(Ogre::Degree(i * 360 / ballsNum));
			node->yaw(Ogre::Degree(rand() * 360. / RAND_MAX));
			demoBalls[i] = node; /// store to animate
		}
		demoBallsMat = bbset->getMaterial();
		demoBallsMat->setFog(true, Ogre::FOG_LINEAR, Ogre::ColourValue(0.058,0.082,0.105), 0., 10., 40.);
		demoBallsMat->setDiffuse(0,0,0,0);
		registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			demoBallsMat->getBestTechnique()->getPass(0), Ogre::ColourValue(0,0,0,1), fadeInTime, &Ogre::Pass::getDiffuse, &Ogre::Pass::setDiffuse)
			);

		/// - base
		Ogre::SceneNode* baseParent = demoCollection->newSceneNode(camera, &Ogre::Vector3(0,0,0));
		baseParent->yaw(Ogre::Degree(rand() * 360. / RAND_MAX)); /// 180 = go behind camera initially
		base = demoCollection->newSceneNode(baseParent, &Ogre::Vector3(0,0,-20));
		base->setScale(0,0,0);
		demoCollection->storeMovable(OgreBuilder::createEntity("cannon_base", "cannon_base.mesh", false, "", base));
		registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			base, Ogre::Vector3(3,3,3), fadeInTime, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
	}

	void Map::hideDemo() {

		static const double fadeOutTime = 0.5;

		TPlaylist* list;

		/// get current values
		Ogre::Pass* pass = demoBallsMat->getBestTechnique()->getPass(0);
		Ogre::ColourValue tempCol = pass->getDiffuse();
		Ogre::Vector3 scale = base->getScale();

		deleteTweens();

		demoBallsMat->setDiffuse(tempCol);
		list = registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			pass, Ogre::ColourValue(0,0,0,0), fadeOutTime, &Ogre::Pass::getDiffuse, &Ogre::Pass::setDiffuse)
			);
		list->id = DEMO_OUT_ID;

		base->setScale(scale);
		list = registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			base, Ogre::Vector3(0,0,0), fadeOutTime, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		list->id = DEMO_OUT_ID;
	}

	// map "demo"
	// => _time = time since last frame
	void Map::runDemoFrame(double _time) {

		static const double timeWrap = 2* Pi / POLAR_TIME;

		/// - camera rotation
		rot.x += _time * (FULL_ROT / DAY_TIME);
		while (rot.x > FULL_ROT) rot.x -= FULL_ROT;

		time += _time;
		while (time > timeWrap) time -= timeWrap;
		rot.y = sin(time * POLAR_TIME) * (YROT_MAX * 2. / 3.);
		while (rot.y > FULL_ROT) rot.y -= FULL_ROT;

		cameraRotWrapper.setOrientation(Ogre::Vector3(rot.x,rot.y,0));

		/// - demoBalls rotation
		int ballsNum = demoBalls.size();
		for (int i = ballsNum - 1; i >= 0; --i) {
			NodeWrapper& node = demoBalls.at(i);
			node.getNode()->roll(Ogre::Degree(2. * _time),Ogre::Node::TS_PARENT);
			node.getNode()->yaw(Ogre::Degree(20. * sqrt(ballsNum - (double)i) / ballsNum * _time),Ogre::Node::TS_LOCAL);
		}

		/// - base rotation
		if (base) {
			base->pitch(Ogre::Degree(20. * _time),Ogre::Node::TS_PARENT);
			base->yaw(Ogre::Degree(85. * _time),Ogre::Node::TS_LOCAL);

			base->getParentSceneNode()->roll(Ogre::Degree(sin(time * POLAR_TIME) * (YROT_MAX / 10.) * _time),Ogre::Node::TS_PARENT);
			base->getParentSceneNode()->yaw(Ogre::Degree(12. * _time),Ogre::Node::TS_LOCAL);
		}
	}

	// main map function
	MapOptions Map::show(const SubMainOptions::FinalChoice& mode, Player* _player) {

		MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);
		mapOptions.reset();

//#ifndef _DEPLOY
		if (hacked) {
			//SoundBoss::getSingletonPtr()->freeAllSounds();

			FANLib::FSLClassDefinition* p3dLevels = P3DScripts::p3dLevels->getRoot();
			int cosmos = p3dLevels->getInt("currentCosmos");
			int level = p3dLevels->getInt("currentLevel");
			mapOptions.mainOption = MapOptions::START_GAME;
			mapOptions.cosmosID = (MapOptions::CosmosID)cosmos;
			mapOptions.level = level;
			return mapOptions;
		}
//#endif

		player = _player;
		earthRoot->setVisible(true);

		selectedEntity = &unselectedEntity;
		if ( player->hasFinishedGame() )
		{
			if ( player->checkJustNow100Percent() ) {
				deleteTweens();
				destroyDemo();
				mapOptions.mainOption = MapOptions::SHOW_END_CREDITS;
				return mapOptions;
			}

			setAllActiveEntities();

		} else if (setActiveEntities())
		{
			/// game has JUST finished, show credits NOW
			deleteTweens();
			destroyDemo();
			mapOptions.mainOption = MapOptions::SHOW_END_CREDITS;
			return mapOptions;
		}

		// open 'mapText' (gui)
		mapText->open();
		static std::string mapTextOut = P3DScripts::p3dVars->getRoot()->getCString("MapTextOut");
		mapText->wEdit->setCaption(mapTextOut);

		notFinished = true;
		zoomedIn = false;
		zoomingIn = false;
		fadingIn = true;

		// "fade-in"
		TPlaylist* list;
		/// - make earth appear
		earthRoot->setScale(0,0,0);
		list = registerTween(new TSin<Ogre::SceneNode, Ogre::Vector3>(
			earthRoot, Ogre::Vector3::UNIT_SCALE, SCALE_TIME, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		list->id = FADE_IN_ID;
		earthWrapper.setOrientation(Ogre::Vector3(180, 0, 0));
		list = registerTween(new TSin<NodeWrapper, Ogre::Vector3>(
			&earthWrapper, Ogre::Vector3(0,0,0), SCALE_TIME, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation)
			);
		list->id = FADE_IN_ID;

		/// - move cameraRot according to player's chapter!
		Ogre::Vector3 camRot = FSLToOgre::vector2ToVector3( P3DScripts::p3dMap->getRoot()->getArray("chapterCamPos")->getClass(player->chapter) );

		list = registerTween(new TSin<NodeWrapper, Ogre::Vector3>(
			&cameraRotWrapper, camRot, SCALE_TIME, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation)
			);
		list->id = FADE_IN_ID;

		OgreAddOns::setOrientationYXZ(mouseRot,camRot.x,camRot.y);
		rot.x = camRot.x;
		rot.y = camRot.y;


		// start rendering!
		HIDBoss::getSingletonPtr()->clippingRectEnabled = false;
		acceptKeyEvents(true);
		Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
		ogreRoot->addFrameListener(this);
		ogreRoot->startRendering();
		// rendering ended!
		ogreRoot->removeFrameListener(this);

		acceptKeyEvents(false);
		acceptMouseEvents(false);
		HIDBoss::getSingletonPtr()->clippingRectEnabled = true;
		mapText->close();

		/// return selections made
		return mapOptions;
	}

	void Map::setAllActiveEntities() {
		activeEntities.clear();
		/// make all Cosmoi entities visible
		for (int i = entities.size() - 1; i >= 0; --i)
		{
			MapEntity& entity = entities.at(i);
			entity.getCenter()->setVisible(true);
			activeEntities.push_back(&entity);
		}
	}

	bool Map::setActiveEntities()
	{
		activeEntities.clear();

		/// activate any remaining entities from current chapter
		if ( !player->hasFinishedGame() )
		{
			const std::vector<MapOptions::CosmosID>& chapterIDs = MapOptions::getChapterCosmoi().at(player->chapter); /// <current> chapter IDs
			FANLib::FSLClass* levels = P3DScripts::p3dLevels->getRoot();
			/// make all Cosmoi entities invisible
			for (int i = entities.size() - 1; i >= 0; --i)
			{
				MapEntity& entity = entities.at(i);
				entity.getCenter()->setVisible(false);

				/// has the player finished this cosmos?
				if (player->cosmoi.at(entity.getID()).size() == levels->getArray(MapOptions::cosmosIDToStr.find(entity.getID())->second.c_str())->getSize()) {
					continue;
				}
				/// if this entity isn't finished, make it visible if it is part of <current> chapter
				for (int j = chapterIDs.size() - 1; j >= 0; --j) {
					if (chapterIDs.at(j) == entity.getID()) {
						entity.getCenter()->setVisible(true);
						activeEntities.push_back(&entity);
						break;
					}
				}
			}
		}

		if (activeEntities.empty()) {
			/// TA DA!!!!! THIS CHAPTER IS COMPLETE!!! Advance to next one...
			if ( !player->hasFinishedGame() ) {
				player->chapter = (MapOptions::Chapter)(player->chapter + 1);
				player->save();
			}

			if ( player->hasFinishedGame() ) {
				/// TA RA TA TZOUM PAM POUM!!!!!!!!
				return true; /// <!!!!!!!!!!!!!> UNBELIEVABLE : WHOLE GAME JUST COMPLETED !!!
			}

			return setActiveEntities(); /// re-run this function for next chapter
		}

		/// still some entities to finish this chapter
		return false;
	}

	void Map::destroyLevelSelection()
	{
		for (unsigned i = 0; i < levelBoxes.size(); ++i)
		{
			P3D::MenuBox* levelBox = levelBoxes.at(i);
			levelBox->delay = 0;
			levelBox->setCallBack(MenuBox::CLOSED, this, &Map::levelBoxClosed);
			levelBox->close();
		}
		levelBoxes.clear();

		MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);
	}
	void Map::levelBoxClosed(void* _levelBox)
	{
		MenuBox* levelBox = static_cast<MenuBox*>( static_cast<FANLib::MCBSystem*>( _levelBox ) ); /// NOTE : double conversion essential
		delete levelBox;
	}

	void Map::createLevelSelection()
	{
		static const Ogre::Vector2 SCREEN = P3DWindow::getSize();
		static const Ogre::Vector2 SIZE( 0.03 * SCREEN.y, 0.03 * SCREEN.y );
		static const Ogre::Vector2 SIZE_SMALL( 0.02 * SCREEN.y, 0.02 * SCREEN.y );
		static const double BOX_DELAY = 0.1;

		const MapOptions::CosmosID id = selectedEntity->getID();
		const std::deque<unsigned> cosmosScores = player->cosmoi.at( id );
		const std::string cosmosID = MapOptions::cosmosIDToStr.find( id )->second;
		const FANLib::FSLArray* fslLevels = P3DScripts::p3dLevels->getRoot()->getArray( cosmosID.c_str() );

		/// rows
		for ( unsigned j = 0; j <= cosmosScores.size() / 4; ++j )
		{
			/// columns
			unsigned columns = cosmosScores.size() - j*4;
			if (columns > 4) columns = 4;
			for ( unsigned i = 0; i < columns; ++i )
			{
				unsigned levelNum = levelBoxes.size();
				std::string levelBoxName = FANLib::Utils::toString( levelNum );
				if ( levelNum < 10 ) levelBoxName = "0" + levelBoxName; /// for easier dereferencing, always 2 chars needed
				MenuBox* levelBox = new MenuBox( "Overlay/MenuExtra", levelBoxName );
				bool expert = (unsigned int)(fslLevels->getClass( levelNum )->getInt( "expert" )) <= cosmosScores.at( levelNum );

				std::string levelNumStr = FANLib::Utils::toString( levelNum+1 );

				if (expert) {
					levelBox->defaultSize = SIZE_SMALL;
					levelBox->setText( "#ffffff"+levelNumStr );
					levelBox->setBorderSize( levelBox->getBorderSize() * SIZE_SMALL/SIZE );
				} else {
					levelBox->defaultSize = SIZE;
					levelBox->setText( "#ff8888"+levelNumStr );
				}
				levelBox->setPosition( Ogre::Vector2( 0.5 * SCREEN.x * P3DWindow::getPixelRatio() + (i - 1.5)*0.25 * SCREEN.y, ( 0.5 + (j - 1.9)*0.16 ) * SCREEN.y ) );
				levelBox->delay = BOX_DELAY * levelBoxes.size();
				levelBox->setCallBack(MenuBox::CLICKED, this, &Map::levelBoxClicked);
				levelBox->open();
				levelBoxes.push_back( levelBox );
			}
		}

		HIDBoss::getSingletonPtr()->setMousePosition(SCREEN.x / 2., SCREEN.y / 2.);
		MyGUI::Gui::getInstancePtr()->setVisiblePointer(true);
	}

	void Map::destroyDemo()
	{
		delete demoCollection;
		demoCollection = 0;
		base = 0;
		demoBalls.clear();
	}

	void Map::unload() {

		entities.clear();		/// <1st> !!! They contain tweens which access 'mapCollection' SceneNodes when deleted
		activeEntities.clear();

		deleteTweens();			/// -2nd- likewise

		//SceneBaseCreator::getSingletonPtr()->setCamera(0);
		delete mapCollection;	/// <3rd>
		mapCollection = 0;

		destroyDemo();

		delete mapText;
		mapText = 0;

		delete hintBox;
		hintBox = 0;

		delete subBlack;
		subBlack = 0;

		SceneBaseCreator::getSingletonPtr()->destroySceneBase();

		Ogre::ResourceGroupManager::getSingletonPtr()->unloadResourceGroup("MAP");
	}

	bool Map::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		static int x,y;
		HIDBoss::getSingletonPtr()->captureDevices();
		HIDBoss::getSingletonPtr()->getMousePosition(x, y);
		MyGUI::InputManager::getInstancePtr()->injectMouseMove(x, y, 0);

		TPlayer::getSingletonPtr()->run(evt.timeSinceLastFrame);

		if (fadingIn || zoomedIn) {
		} else {
			followMouse();
		}

		return notFinished;
	}

	void Map::followMouse() {
		static double factor = 0;
		static const double accFactor = 0.02;
		static const double decFactor = 0.002;
		static const double maxFactor = 0.5;
		static const double tolerance = 0.01;

		Ogre::Quaternion source = cameraRot->getOrientation();
		Ogre::Quaternion dest = mouseRot->getOrientation();
		double difference = ( source - dest ).Norm();
		if (difference > 1) difference = 4 - difference; /// <!!!>

		if ( difference > tolerance ) {
			factor += accFactor * difference;
			if (factor > maxFactor) factor = maxFactor;
		} else {
			factor -= decFactor;
			if (factor < 0.) factor = 0.;
		}
		cameraRot->setOrientation( Ogre::Quaternion::Slerp(factor, source, dest, true) );
	}

	void Map::onMouseMove() {

		if (zoomedIn) return;

		static const double mouseSpeed = 1./8.;

		int xr, yr;
		HIDBoss::getSingletonPtr()->getMouseMove(xr, yr);

		rot.x += xr * mouseSpeed;

		rot.y += yr * mouseSpeed;
		rot.y = FANLib::Math::bound(rot.y, YROT_MAX);

		/// set new orientation
		OgreAddOns::setOrientationYXZ(mouseRot, rot.x, rot.y);
		std::cout << rot.x << " " << rot.y << "\n";

		/// check closest <map entity> to mouse
		MapEntity* closest = &unselectedEntity;
		double minDistance = minimumDistance;
		for (unsigned int i = 0; i < activeEntities.size(); ++i) {
			double distance = activeEntities.at(i)->getCenter()->_getDerivedPosition().squaredDistance( mouse->_getDerivedPosition() );
			if (minDistance > distance) {
				minDistance = distance;
				closest = activeEntities.at(i);
			}
		}

		selectEntity(closest);
	}

	void Map::selectEntity(MapEntity* closest) {
		/// animate newly selected entity
		if (selectedEntity != closest) {
			static const double SCALE_TIME = 0.5;
			static const double SCALE_NORM = 1.;
			static const double SCALE_SEL = 1.2;

			double currentScale;

			/// scale back previously selected entity
			currentScale = selectedEntity->getScale();
			selectedEntity->deleteTweens();
			selectedEntity->setScale(currentScale);
			selectedEntity->registerTween(new TLinear<MapEntity, double>(
				selectedEntity, SCALE_NORM, SCALE_TIME, &MapEntity::getScale, &MapEntity::setScale)
				);

			selectedEntity = closest;

			/// scale newly selected entity
			currentScale = selectedEntity->getScale();
			selectedEntity->deleteTweens();
			selectedEntity->setScale(currentScale);
			selectedEntity->registerTween(new TLinear<MapEntity, double>(
				selectedEntity, SCALE_SEL, SCALE_TIME, &MapEntity::getScale, &MapEntity::setScale)
				);

		}
	}

	void Map::gameOn(void *) {
		notFinished = false;
	}

	// mouse clicked
	void Map::onMouseChange() {

		if (zoomedIn) {

			if ( HIDBoss::getSingletonPtr()->isButtonPressed(HIDBoss::MOUSE_LEFT) ) {

				if ( !player->hasFinishedGame() )
				{
					mapOptions.mainOption = MapOptions::START_GAME;
					mapOptions.cosmosID = selectedEntity->getID();

					subBlack->open();
					Ogre::Camera* cam = SceneBaseCreator::getSingletonPtr()->getCamera();
					registerTween(new TCos<Ogre::Camera, Ogre::Radian>(
						cam, cam->getFOVy() / 1.5, subBlack->fadeTime, &Ogre::Camera::getFOVy, &Ogre::Camera::setFOVy)
						);

					acceptMouseEvents(false); /// choice finalized
				}

			} else if ( !zoomingIn && HIDBoss::getSingletonPtr()->isButtonPressed(HIDBoss::MOUSE_RIGHT) ) {

				/// zoom-out!
				static std::string mapTextOut = P3DScripts::p3dVars->getRoot()->getCString("MapTextOut");
				mapText->wEdit->setCaption(mapTextOut);

				acceptMouseEvents(false);
				TPlaylist* list;

				list = registerTween(new TLinear<Map, Ogre::Vector3>(
					this, orbitPos, ZOOM_TIME, &Map::getCamPos, &Map::setCamPos)
					);
				list->id = ZOOM_OUT_ID;

				oriFactor = 0.;
				destOri = orbitOrient;
				list = registerTween(new TSin<Map, double>(
					this, FACTOR_END, ZOOM_TIME, &Map::getOriFactor, &Map::setOriFactor)
					);
				list->id = ZOOM_OUT_ID;

				// is game already finished?
				if ( player->hasFinishedGame() ) {
					destroyLevelSelection();
				} else {
					hintBox->close();
				}
			}

		} else {

			if (HIDBoss::getSingletonPtr()->isButtonPressed(HIDBoss::MOUSE_LEFT) && selectedEntity->getID() != MapOptions::UNDEFINED) {

				/// zoom-in!
				zoomedIn = true;
				zoomingIn = true;
				mBBSet->setVisible(false);
				TPlaylist* list;

				orbitPos = camera->_getDerivedPosition(); /// save
				list = registerTween(new TSin<Map, Ogre::Vector3>(
					this, selectedEntity->getCamera()->_getDerivedPosition(), ZOOM_TIME, &Map::getCamPos, &Map::setCamPos)
					);
				list->id = ZOOM_IN_ID;

				orbitOrient = getCamOri(); /// save
				//std::cout<<"orbitOrient ="<<orbitOrient<<"\n";
				oriFactor = 0.;
				destOri = selectedEntity->getCamera()->_getDerivedOrientation();
				list = registerTween(new TSin<Map, double>(
					this, FACTOR_END, ZOOM_TIME * 1.5, &Map::getOriFactor, &Map::setOriFactor)
					);
				list->id = ZOOM_IN_ID;

				static std::string mapTextIn = P3DScripts::p3dVars->getRoot()->getCString("MapTextIn");
				mapText->wEdit->setCaption(mapTextIn);

				// is game already finished?
				if ( player->hasFinishedGame() ) {
					createLevelSelection();
				} else {
					hintBox->setText(getHintBoxStr());
					hintBox->open();
				}

			} else if ( HIDBoss::getSingletonPtr()->isButtonPressed(HIDBoss::MOUSE_RIGHT) )
			{
				// go back to Menu
				mapOptions.mainOption = MapOptions::GOTO_MENU;
				notFinished = false;
				selectEntity(&unselectedEntity);

				// "fade-out"
				TPlaylist* list;
				/// - make earth disappear
				list = registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
					earthRoot, Ogre::Vector3::ZERO, SCALE_TIME, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
					);
				list = registerTween(new TCos<NodeWrapper, Ogre::Vector3>(
					&earthWrapper, Ogre::Vector3(-180,0,0), SCALE_TIME, &NodeWrapper::getOrientation, &NodeWrapper::setOrientation)
					);

				double factor = rot.y / (YROT_MAX * 2. / 3.);
				if (factor > 1) {
					factor = 1;
				} else if (factor < -1) {
					factor = -1;
				}
				time = asin(factor) / POLAR_TIME;
			}
		}
	}

	void Map::levelBoxClicked(void* _levelBox)
	{
		MenuBox* levelBox = static_cast<MenuBox*>( static_cast<FANLib::MCBSystem*>( _levelBox ) ); /// NOTE : double conversion essential
		mapOptions.mainOption = MapOptions::START_GAME;
		mapOptions.cosmosID = selectedEntity->getID();
		FANLib::Utils::fromString( mapOptions.level, levelBox->getName().substr(0,2) );
		destroyLevelSelection();
		subBlack->open();
	}

	const std::string& Map::getHintBoxStr()
	{
		static std::string Area = P3DScripts::p3dVars->getRoot()->getCString("Area");
		static std::string Difficulty = P3DScripts::p3dVars->getRoot()->getCString("Difficulty");
		static std::string Complete = P3DScripts::p3dVars->getRoot()->getCString("Complete");
		static std::string str;

		MapOptions::CosmosID id = selectedEntity->getID();
		std::string area = MapOptions::cosmosIDToStr.find(id)->second;
		std::string areaName = MapOptions::cosmosIDToName().find(id)->second;
		FANLib::FSLClass* levels = P3DScripts::p3dLevels->getRoot();

		str = Area + " : " + areaName + "\n\n";

		str += Difficulty + " : ";
		str += levels->getArray("diffs")->getCString(id);
		str += "\n\n";

		str += Complete + " : ";
		str += "#ffa0a0" + FANLib::Utils::toString( player->cosmoi.at(id).size() ) + "#ffffff / ";
		str += FANLib::Utils::toString( levels->getArray(area.c_str())->getSize() );

		return str;
	}

	void Map::playlistFinished(TPlaylist* list) {

		this->TPlayed::playlistFinished(list);

		switch (list->id) {

			case DEMO_OUT_ID:
				if (existsID(DEMO_OUT_ID)) return;
				destroyDemo();
				break;

			case ZOOM_OUT_ID:
				if (existsID(ZOOM_OUT_ID)) return;
				acceptMouseEvents(true);
				zoomedIn = false;
				mBBSet->setVisible(true);
				break;

			case FADE_IN_ID:
				if (existsID(FADE_IN_ID)) return;
				acceptMouseEvents(true);
				fadingIn = false;
				break;

			case ZOOM_IN_ID:
				if (existsID(ZOOM_IN_ID)) return;
				//acceptMouseEvents(true);
				zoomingIn = false;
				break;
		}
	}

	void Map::onKeyEvent() {
		if (HIDBoss::getSingletonPtr()->isKeyPressed(OIS::KC_ESCAPE)) {
			mapOptions.mainOption = MapOptions::QUIT_APP;
			notFinished = false;
		}
	}

	const Ogre::Vector3& Map::getCamPos() const {
		return camera->_getDerivedPosition();
	}
	void Map::setCamPos(const Ogre::Vector3& pos) {
		OgreAddOns::setDerivedPosition(camera, &pos);
	}

	void Map::setOriFactor(const double& f) {
		oriFactor = f;
		if (oriFactor == FACTOR_END) {
			setCamOri( destOri );
		} else {
			setCamOri( Ogre::Quaternion::Slerp(oriFactor, getCamOri(), destOri, true) );
		}
	}

	const Ogre::Quaternion& Map::getCamOri() const {
		return camera->_getDerivedOrientation();
	}
	void Map::setCamOri(const Ogre::Quaternion& ori) {
		camera->setOrientation(	(camera->getParentSceneNode()->_getFullTransform().inverse() * ori).extractQuaternion() );
	}
}