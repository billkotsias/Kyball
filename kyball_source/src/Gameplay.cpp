#include "StdAfx.h"
#include "game\Gameplay.h"

#include "P3DApp.h"		/// have I forgoten anything else to include :(
#include "P3DWindow.h"	/// yes, this one :(
#include "OgreAddOns.h"	/// ...this one
#include "SceneBaseCreator.h"
#include "ShadowManager.h"
#include "BurstBoss.h"
#include "visual\VisualBoss.h"
#include "visual\FlyIn.h"
#include "tween\TPlayer.h"

#include "menu\Player.h"
#include "game\cannon\Cannon.h"
#include "game\Game.h"
#include "game\CosmosCreator.h"
#include "game\LevelBuilder.h"
#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\GameplayCollection.h"
#include "game\ai\AI.h"
#include "game\ai\AIPVAnimator.h"
#include "game\form\Form.h"
#include "game\rule\RuleManager.h"
#include "game\action\Action.h"
#include "game\animal\Animal.h"
#include "game\event\EventManager.h"
#include "game\event\LostEvent.h"
#include "game\event\WonEvent.h"
#include "game\event\CosmosFinEvent.h"
#include "collision\CollisionParams.h"
#include "hid\HIDBoss.h"
#include "sound\SoundBoss.h"
#include "sound\Sound.h"

#include "game\editor\BallEditor.h"

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreEntity.h>
#include <OgreNode.h>

#include <vector>
#include <core\Utils.h>

#include "gui\HintBox.h"
#include "gui\LevelShow.h"
#include "gui\SubScore.h"
#include "gui\GameMenu.h"
#include "gui\SubPaused.h"
#include "gui\SubWin.h"
#include "gui\SubLose.h"

// temp ??!??
#include "P3DScripts.h"
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>
#include <fsl\FSLArray.h>

// debug
#include <OIS/OIS.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
#include "game\Camera.h"

namespace P3D {

	const double Gameplay::WIN_TIME = 4.4;
	const double Gameplay::LOSE_TIME = 2.9;
	
	Gameplay::Gameplay() : gameplayResult(Rule::NOTHING), disableID(FANLib::DisableID::get()), bEdit(0) {

		levelBuilder = new LevelBuilder();
	}

	Gameplay::~Gameplay() {
		delete levelBuilder;
	}

	void Gameplay::start(void* queue) {
		enableAllCannons();
		initScores();
		gameTime = 0.;
		started = true;
		((Queue*)queue)->next(); /// I have finished already, next!
	}

	void Gameplay::adventure(MapOptions mapOptions, GameplayBase* base) {

		static const char* const CALLER = "Gameplay\\";
		static const char* const DEBUG_OV = "Core/DebugOverlay";

		// sounds
		Sound::GAME_LOSE->init();
		Sound::GAME_WIN->init();

		/* register gameplay bases to visit every cycle */;
		gameplayBases.push_back(base);

		//HIDBoss::getSingletonPtr()->setMouseGrabbed(true);

//#ifndef _DEPLOY
//#ifdef _DEBUG
		Ogre::Overlay* debugOV = Ogre::OverlayManager::getSingleton().getByName(DEBUG_OV);
		debugOV->show();
//#endif
		FlyIn flyIn(base); /// added in queue if 1st level is to be played; keep this definition order...!

		Queue queue;
		hintBox = new HintBox("Overlay/HintBox");
		levelShow = new LevelShow();

		gameMenu = new GameMenu();
		gameMenu->FANLib::MCBSystem::setCallBack(GameMenu::MAP, this, &Gameplay::gotoMap);
		gameMenu->FANLib::MCBSystem::setCallBack(GameMenu::MENU, this, &Gameplay::gotoMenu);
		gameMenu->FANLib::MCBSystem::setCallBack(GameMenu::QUIT, this, &Gameplay::quitNow);

		subPause = new SubPaused(CALLER);
		subPause->FANLib::CallBackSystem::setCallBack(this, &Gameplay::pauseEvent);
		subPause->FANLib::MCBSystem::setCallBack(SubPaused::RETRY, this, &Gameplay::lost);

		subWin = new SubWin(CALLER);
		subWin->FANLib::MCBSystem::setCallBack(SubWin::OPEN, this, &Gameplay::winOpen);
		subWin->FANLib::MCBSystem::setCallBack(SubWin::NEXT, this, &Gameplay::won);

		subLose = new SubLose(CALLER);
		subLose->FANLib::MCBSystem::setCallBack(SubLose::OPEN, this, &Gameplay::openGameMenu);
		subLose->FANLib::MCBSystem::setCallBack(SubLose::NEXT, this, &Gameplay::lost);
		subLose->FANLib::MCBSystem::setCallBack(SubLose::SKIP, this, &Gameplay::skip);

		currentCosmosLevels = P3DScripts::p3dLevels->getRoot()->getArray(base->getCosmos().c_str());
		Player* player = base->player;

		SoundBoss::getSingletonPtr()->newStreamMusic(mapOptions.getCosmosStr() + P3DApp::PATH_DELIMITER + mapOptions.getCosmosStr());

#ifdef _DEPLOY
		currentLevel = 0;
#else
		currentLevel = -1;
#endif

		do {

			/// --------
			/// create level and initialize queue
			queue.reset();

			/// - define level to be played
#ifdef _DEPLOY
			if (mapOptions.getLevel() >= 0) {
				currentLevel = mapOptions.getLevel();
				if (player->hasFinishedGame()) mapOptions.reset(); /// only used 1st time around (if not has finished, it's a hack!!!)
			} else {
				unsigned newLevel = player->getNextLevel(base->getCosmosID(), currentLevel);
				if (newLevel != -1) currentLevel = newLevel; /// else, keep current one!
			}
#else
			if (mapOptions.getLevel() >= 0) {
				currentLevel = mapOptions.getLevel();
				mapOptions.reset(); /// debug
			} else {
				++currentLevel; /// debug
			}
			if (currentLevel >= currentCosmosLevels->getSize()) currentLevel = currentCosmosLevels->getSize() - 1;
#endif

			/// - check if "fly-in" goes into queue
#ifdef _DEPLOY
			if (currentLevel == 0 && !player->hasFinishedGame()) {
#else
			if (currentLevel == 0) {
#endif
				queue.pushBack(&flyIn, &FlyIn::start);
			}

			/// - build level
			GameplayCollection* level = (GameplayCollection*)levelBuilder->build(currentLevel, base, &queue, hintBox);
			base->score = 0; /// reset score to zero
			subWin->expertScore = currentCosmosLevels->getClass(currentLevel)->getInt("expert");

			/// - push "LevelShow" into queue
			levelShow->setText(FANLib::Utils::toString(currentLevel+1));
			queue.pushBack(levelShow, &LevelShow::open);

			/// - push myself into queue
			queue.pushBack(this, &Gameplay::start);

			/// --------
			/// - rest gameplay parameters init
			disableAllCannons();

			/// enable <editor>
//#ifndef _DEPLOY
#ifdef _DEBUG
			bEdit = new BallEditor(base);
#endif
			/// reset gameplay parameters
			gameplayResult = Rule::NOTHING;
			exitStatus = UNDEFINED;
			started = false;
			paused = false;
			pausedFading = false;

			base->getAnimal()->start(); /// start animal
			queue.next(); /// start queue

			/// start rendering loop : will end if <level is finished>, or application <window is closed>
			HIDBoss::getSingletonPtr()->clippingRectEnabled = false;
			acceptKeyEvents(true);
			acceptMouseEvents(true);
			Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
			ogreRoot->addFrameListener(this);

			{
				/// run one frame before rendering starts
				Ogre::FrameEvent runOnce;
				runOnce.timeSinceLastFrame = 1./P3DApp::CYCLES_PER_SEC;
				frameRenderingQueued(runOnce);
			}
			ogreRoot->startRendering();

			ogreRoot->removeFrameListener(this);
			acceptMouseEvents(false);
			acceptKeyEvents(false);
			HIDBoss::getSingletonPtr()->clippingRectEnabled = true;

			/// disable <editor>
			delete bEdit;
			bEdit = 0;

			/// check under which situation the level has ended (check a class variable e.g <enum> "exitGameplayStatus")
			/// - if window closed, save player data and leave application (=> by returning this <ExitStatus> to <caller>)

			/// - if it's NOT too slow, we'll always build next level, even if the player is repeating the same one because he lost
			/// - if it's slow, I'll have to "reset" the level to it's start-up state, maybe <Level> will deal with this : Level->restart()
			/// - FOR NOW I THINK OGRE DOESN'T WORK THIS WAY, SO A RE-BUILD IS ON OUR WAY!

			/* clean-up level */;
			base->getRuleManager()->clearRules();
			base->eraseGameplayObjects(&(level->gameplayObjects)); /// unregister 'level's <gameplayObjects>
			delete level; /// even if player will play the same one again!

			/// <exitStatus> must have been set by this point; if not, we'll play the next available level

		} while (exitStatus == UNDEFINED && gameplayResult != Rule::QUIT);

		gameplayBases.pop_back(); /// empty

		delete subLose;
		delete subWin;
		delete subPause;
		delete gameMenu;
		delete hintBox;
		delete levelShow;

//#ifndef _DEPLOY
////#ifdef _DEBUG
		debugOV->hide();
//#endif
		// HIDBoss::getSingletonPtr()->setMouseGrabbed(false); /* un-capture mouse */;

		// <= possible return values from 'adventure' (good-old switch)
		//	  "gotoMap"
		//	  "gotoMenu"
		//	  "endApp"
	}

	// FrameListener
	/// TODO : set <exitStatus> when returning <false>
	bool Gameplay::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		/// TODO : is this the right place for this?
		if(P3DWindow::getWindow()->isClosed()) {
			exitStatus = END_APP;
			return false;
		}

		/// calculate extra cycles needed for gameplay to catch-up, due to slow frame rate
		static double accumulatedTime = 0;
		static const int MAX_CYCLES = P3DApp::CYCLES_PER_SEC / 6; /// slow gameplay down when we have very low fps (when fps < 6 !)
		accumulatedTime += evt.timeSinceLastFrame;
		int cycles = accumulatedTime * P3DApp::CYCLES_PER_SEC;
		accumulatedTime -= (double)cycles * (1. / P3DApp::CYCLES_PER_SEC);
		if (cycles > MAX_CYCLES) cycles = MAX_CYCLES;

		/// run <gameplay> * cycles missed since last frame
		int gameplayCycles = cycles;
		while (gameplayCycles > 0) {

			for (int i = gameplayBases.size() - 1; i >= 0; --i) {

				/// <TODO> : have some sort of interaction between <GameplayBases>; maybe with a common GameplayObject..? (nah)
				GameplayBase* base = gameplayBases[i];
				Rule::Result result = gameplay(base);

				if (gameplayResult == Rule::NOTHING && started) {

					if (result != Rule::NOTHING) started = false;

					switch (result) {
						case Rule::WIN:
							win(base, WIN_TIME);
							break;
						case Rule::LOSE:
							lose(base, LOSE_TIME);
							break;
					}

				} else if (gameplayResult != Rule::NOTHING) {
					return false; /// <gameplay end>
				}
			}

			--gameplayCycles;
		}

		/// run "once-per-frame" code
		{
			double timeToRun = cycles * (1. / P3DApp::CYCLES_PER_SEC); /// in secs

			/// - gameTime
			if (!paused && started) gameTime += timeToRun;

			/// - TPlayer
			TPlayer::getSingletonPtr()->run(timeToRun);

			/// - VisualBoss (how much I dislike this class :-)
			VisualBoss::getSingletonPtr()->run(timeToRun);

			for (int i = gameplayBases.size() - 1; i >= 0; --i) {

				GameplayBase* base = gameplayBases[i];

				/// - BurstsBoss : clean-up finished
				base->getBurstBoss()->checkInactive();

				/// - Animal
				base->getAnimal()->run(timeToRun);

				/// - Aim
				base->getCannon()->runAim(cycles);

				/// - ShadowManager : must be last visual element
				base->getShadowManager()->update();

				/// - Score
				base->getSubScore()->run(timeToRun);
			}
		}

		/// when all gameplay objects have moved, we MUST call all BillboardSet->_updateBounds(); ! ALL !!!
		/// <BUT THAT'S SOMETHING BillboardSet owners should do THEMSELVES !!!>
		// was:
		//for (int i = gameplayBases.size() - 1; i >= 0; --i) {
		//	gameplayBases[i]->getBBBSet()->_updateBounds();
		//}

		// semi-debug-mode functions
		debugUpdateStats();

		return true;
	}

	// actual gameplay
	Rule::Result Gameplay::gameplay(GameplayBase* base) {

		/// "run" sound
		SoundBoss::getSingletonPtr()->update();

		/// "run" <cannon> "output"
		base->getCannon()->run(1. / P3DApp::CYCLES_PER_SEC); /// instead of "running" the 'Cannon' separately, it could be turned into a fine <GameplayObject>..?

		/// "run" all inputs (includes <cannon> "input")
		HIDBoss::getSingletonPtr()->captureDevices();
		if (paused) {
			static int mx,my;
			HIDBoss::getSingletonPtr()->getMousePosition(mx, my);
			MyGUI::InputManager::getInstancePtr()->injectMouseMove(mx, my, 0);
		}

		/// "run" AIPVAnimator
		base->getAIPVAnimator()->run();


		/// "run" <gameplay objects>
		GameplayObjectsIterator gameIterator;
		GameplayObject* gameplayObject;

		/// - run a game cycle
		std::vector<GameplayObject*>& causeCollision = base->getCauseCollision();
		std::vector<GameplayObject*>& acceptCollision = base->getAcceptCollision();
		std::vector<GameplayObject*>& hybridCollision = base->getHybridCollision();

		std::map<CollisionParams, bool> collisions;		/// <NOTE> : these are hopefully always ordered in <ascending> order
		std::vector<CollisionParams> occuring;			/// collisions actually occuring
		std::vector<CollisionParams> lastCollisions;	/// check if we have the same collisions from previous cycle
		double nearest;									/// time indicating 1st occuring collision or remaining cycle time

		double time = 1.; /// remaining current-cycle time
		do {
			base->groupCollisionObjects();

			collisions.clear();
			occuring.clear();
			/// lastCollisions follows 'occuring'

			/// - if collisions may occur, <cache collision parameters> of all objects
			if (causeCollision.size() + hybridCollision.size()) base->prepareCollisionObjects();

			/// - check for all collisions
			for (int j = causeCollision.size() - 1; j >= 0; --j) {
				GameplayObject* colObj = causeCollision.at(j);

				/// - first check against "causing"
				for (int i = j - 1; i >= 0; --i) {
					checkCollision(colObj, causeCollision.at(i), collisions);
				}
				/// - now check against "accepting"
				for (int i = acceptCollision.size() - 1; i >= 0; --i) {
					checkCollision(colObj, acceptCollision.at(i), collisions);
				}
			}
			for (int j = hybridCollision.size() - 1; j >= 0; --j) {
				GameplayObject* colObj = hybridCollision.at(j);
				for (int i = causeCollision.size() - 1; i >= 0; --i) checkCollision(colObj, causeCollision.at(i), collisions);
				for (int i = acceptCollision.size() - 1; i >= 0; --i) checkCollision(colObj, acceptCollision.at(i), collisions);
			}

			/// - separate 1st occuring collisions, removing those repeated from previous iteration
			nearest = time;
			for (std::map<CollisionParams, bool>::iterator it = collisions.begin(); it != collisions.end(); ++it) {
				const CollisionParams& params = it->first;
				if (params.time <= nearest) {
					if (FANLib::Utils::rFind(lastCollisions, lastCollisions.size(), params) < 0) { /// reject collisions repeated from previous iteration!!!
						occuring.push_back(params);
						nearest = params.time;
					}
				} else {
					break; /// passed over occuring collisions!
				}
			}
			lastCollisions = occuring;

			/// - move all objects up to "minimum" time : their <collision params> are now invalid!
			if (gameIterator.begin(base, gameplayObject)) {
				while (gameIterator.next()) {
					gameplayObject->ai->run(nearest);
				}
			}

			/// - exchange collision effect!
			for (int i = occuring.size() - 1; i >= 0; --i) {

				CollisionParams& params = occuring.at(i);
				params.calculateLambda();

				GameplayObject* obj1 = params.gameObj1;
				GameplayObject* obj2 = params.gameObj2;

				obj1->receiveAction(obj2->ai->getAction()->copy()->embedCollision(params)); /// a <copy> is made for the <GameplayObjects>
				obj2->receiveAction(obj1->ai->getAction()->copy()->embedCollision(params.swap()));
			}

			/// - tell all objects to react to their incoming events
			if (gameIterator.begin(base, gameplayObject)) {
				while (gameIterator.next()) {
					gameplayObject->react();
				}
			}

			/// - end of sub-cycle
			time -= nearest;
		} while (time > 0);

		/// execute <end-of-a-cycle> code
		if (gameIterator.begin(base, gameplayObject)) {
			while (gameIterator.next()) gameplayObject->ai->endOfCycle();
		}

		/// inform objects of future collisions, from nearest to furthest ones
		for (std::map<CollisionParams, bool>::iterator it = collisions.begin(); it != collisions.end(); ++it) {
			const CollisionParams& params = it->first;
			if (params.time <= nearest) continue; /// already occured
			params.gameObj1->ai->futureAction(params.gameObj2->ai->getAction()->embedCollision(params)); /// <AI> will decide if it needs to keep a <copy>
			params.gameObj2->ai->futureAction(params.gameObj1->ai->getAction()->embedCollision(params.swap()));
		}

		/// <delete> any marked 'GameplayObjects'
		if (gameIterator.begin(base, gameplayObject)) {
			while (gameIterator.next()) {
				if (gameplayObject->dead) {
					delete gameplayObject;
					gameIterator.removeGameplayObject();
				}
			}
		}

		/// <update> 'GameplayObjects' visually
		if (gameIterator.begin(base, gameplayObject)) {
			while (gameIterator.next()) {
				gameplayObject->update();
			}
		}

		return base->getRuleManager()->checkRules(); /// check if this player/base has <WON> or <LOST>
	}

	void Gameplay::checkCollision(GameplayObject* obj1, GameplayObject* obj2, std::map<CollisionParams, bool>& collisions) {

		CollisionParams params = obj1->checkCollision(obj2);

		if (params.isValid()) {
			params.gameObj1 = obj1;
			params.gameObj2 = obj2;
			collisions[params] = true;
		}
	}

	void Gameplay::disableAllCannons(bool full) {
		for (int i = gameplayBases.size() - 1; i >= 0; --i) {
			Cannon* cannon = gameplayBases[i]->getCannon();
			cannon->disable(disableID);
			if (full) cannon->acceptMouseEvents(false);
		}
	}
	void Gameplay::enableAllCannons(bool full) {
		for (int i = gameplayBases.size() - 1; i >= 0; --i) {
			Cannon* cannon = gameplayBases[i]->getCannon();
			cannon->enable(disableID);
			if (full) cannon->acceptMouseEvents(true);
		}
	}

	void Gameplay::disableAllCameras() {
		for (int i = gameplayBases.size() - 1; i >= 0; --i) {
			gameplayBases[i]->getCamera()->disable(disableID);
		}
	}
	void Gameplay::enableAllCameras() {
		for (int i = gameplayBases.size() - 1; i >= 0; --i) {
			gameplayBases[i]->getCamera()->enable(disableID);
		}
	}

	void Gameplay::initScores() {
		for (int i = gameplayBases.size() - 1; i >= 0; --i) {
			gameplayBases[i]->score = 0;
			SubScore* subScore = gameplayBases[i]->getSubScore();
			subScore->score = 0;
			subScore->open();
		}
	}

	void Gameplay::win(GameplayBase* base, double delay) {

		if (paused) {
			closeGameMenu();
			subPause->close();
		}

		/// calculate <time bonus> - <TODO> : THIS SHOULD BE IN A RULE, NOT HERE, FOR GOD'S SHAKE!
		int timeBonus = 120000 - 10000 * sqrt(gameTime); /// = 0 at time = 144 secs
		if (timeBonus < 0) timeBonus = 0;

		MapOptions::CosmosID cosmosID = base->getCosmosID();

		/// update high scores
		base->score += timeBonus;
		Player* player = base->player;
		player->setScore(cosmosID, currentLevel, base->score); // this is where "current level" is advanced!
		//player->earnBadges(); /// <TODO> : show window with newly earned badges !!!
		player->save();

		/// update "winning" panel
		subWin->timeBonus = timeBonus;
		subWin->totalScore = base->score;
		subWin->delay = delay;
		if (base->score >= subWin->expertScore) {
			/// EXPERT SCORE! RULE HERE TOO, NO?
			base->getCannon()->playFireworks();
		}

		/// produce according 'win' effect
		base->getAnimal()->event(Animal::win);
		/// - is this normal or <great> win?
		if (
			( !player->hasFinishedGame() && player->getNextLevel(cosmosID) >= currentCosmosLevels->getSize() ) ||
			( player->isExpertAt(cosmosID) ) )
		{
			/// <great> win
			exitStatus = GOTO_MAP;
			base->getEventManager()->incomingEvent(new CosmosFinEvent());
			Sound::GAME_WIN_GREAT->play();
			subWin->open(SubWin::COSMOS);
		} else {
			/// normal win
			base->getEventManager()->incomingEvent(new WonEvent());
			Sound::GAME_WIN->play();
			subWin->open(SubWin::LEVEL);
		}

		disableAllCannons();
	}

	void Gameplay::winOpen(void*) {
		openGameMenu();
	}

	void Gameplay::won(void*) {
		closeGameMenu();
		gameplayResult = Rule::WIN;	/// simply ends gameplay loop
	}

	void Gameplay::skip(void*) {
		closeGameMenu();
		gameplayResult = Rule::WIN;	/// simply ends gameplay loop

		// we are obviously talking about Player 1!
		GameplayBase* base = gameplayBases[0];
		Player* player = base->player;
		player->setScore(base->getCosmosID(), currentLevel, 0); // this is where "current level" is advanced! ZERO POINTS!
		player->save();
		if ( player->getNextLevel(base->getCosmosID(), currentLevel ) >= currentCosmosLevels->getSize() ) {
			exitStatus = GOTO_MAP;
		}
		++currentLevel; /// this is for after-game-finished skipping
	}

	void Gameplay::lose(GameplayBase* base, double delay) {

		if (paused) {
			closeGameMenu();
			subPause->close();
		}

		base->getAnimal()->event(Animal::lose);
		base->getEventManager()->incomingEvent(new LostEvent());
		Sound::GAME_LOSE->play();

		disableAllCannons();

		subLose->delay = delay;
		subLose->open();
	}

	void Gameplay::lost(void*) {
		closeGameMenu();
		gameplayResult = Rule::LOSE; /// simply ends gameplay loop
	}

	void Gameplay::quitNow(void*) {
		gameplayResult = Rule::QUIT;
		exitStatus = END_APP;
	}

	void Gameplay::gotoMap(void*) {
		gameplayResult = Rule::QUIT;
		exitStatus = GOTO_MAP;
	}

	void Gameplay::gotoMenu(void*) {
		gameplayResult = Rule::QUIT;
		exitStatus = GOTO_MENU;
	}

	//
	// more functions
	//

	void Gameplay::openGameMenu(void*) {
		HIDBoss::getSingletonPtr()->clippingRectEnabled = true;
		MyGUI::Gui::getInstancePtr()->setVisiblePointer(true);
		Ogre::Vector2 screenSize = P3DWindow::getSize();
		HIDBoss::getSingletonPtr()->setMousePosition(screenSize.x/2., screenSize.y*2./3.);

		disableAllCannons(true); /// full disable
		disableAllCameras();
		gameMenu->open();
		paused = true;
	}

	void Gameplay::closeGameMenu(void*) {
		HIDBoss::getSingletonPtr()->clippingRectEnabled = false;
		MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);
		enableAllCannons(true); /// full re-enable
		enableAllCameras();
		gameMenu->close();
		paused = false;
	}

	void Gameplay::togglePause() {
		if (!started || pausedFading) return;

		if (!paused) {
			openGameMenu();
			subPause->open();
			pausedFading = true; /// prevent keyboard auto-repeating
		} else {
			closeGameMenu();
			subPause->close();
			pausedFading = true;
		}
	}

	void Gameplay::onMouseChange() {

		if (!paused) return;

		static int mx,my;

		HIDBoss* hid = HIDBoss::getSingletonPtr();
		HIDBoss::getSingletonPtr()->getMousePosition(mx, my);
		MyGUI::InputManager* min = MyGUI::InputManager::getInstancePtr();

		if (hid->isButtonPressed(HIDBoss::MOUSE_LEFT)) {
			min->injectMousePress(mx, my, MyGUI::MouseButton::Left);
		}
		if (hid->isButtonReleased(HIDBoss::MOUSE_LEFT)) {
			min->injectMouseRelease(mx, my, MyGUI::MouseButton::Left);
		}
	}

	void Gameplay::onKeyEvent() {

		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		Ogre::Camera* camera = sceneBase->getCamera();

		HIDBoss* hid = HIDBoss::getSingletonPtr();

		/// open <game menu>
		if(hid->isKeyPressed(OIS::KC_ESCAPE) || hid->isKeyPressed(OIS::KC_P)) {
			togglePause();
		}

#ifndef _DEPLOY
//#ifdef _DEBUG

		/// move camera
		//static const double speed = 0.05 * 60 / P3DApp::CYCLES_PER_SEC;
		//static const Ogre::Radian speedRot(0.01 * 60 / P3DApp::CYCLES_PER_SEC);
		//Ogre::Vector3 camMove = Ogre::Vector3::ZERO;
		//Ogre::Radian camRotY(0);
		//if(keyboard->isKeyDown(OIS::KC_UP))		camMove.z = -speed;
		//if(keyboard->isKeyDown(OIS::KC_DOWN))	camMove.z += speed;
		//if(keyboard->isKeyDown(OIS::KC_PGUP))	camMove.y = speed;
		//if(keyboard->isKeyDown(OIS::KC_PGDOWN))	camMove.y -= speed;
		//if(keyboard->isKeyDown(OIS::KC_LEFT))	camRotY = speedRot;
		//if(keyboard->isKeyDown(OIS::KC_RIGHT))	camRotY -= speedRot;
		//camera->yaw(camRotY);
		//camera->moveRelative(camMove);

		// debug animal
		static const double trSp = 0.01*100.;
		static const double scSp = 0.95;
		static const double roSp = 1;

		Animal* animal = gameplayBases[0]->getAnimal();
		/// animal event debug
		if(hid->isKeyPressed(OIS::KC_Q)) animal->event(Animal::look);
		if(hid->isKeyPressed(OIS::KC_W)) animal->event(Animal::tick1);
		if(hid->isKeyPressed(OIS::KC_E)) animal->event(Animal::tick2);
		if(hid->isKeyPressed(OIS::KC_R)) animal->event(Animal::tick3);
		if(hid->isKeyPressed(OIS::KC_1)) animal->event(Animal::happy1);
		if(hid->isKeyPressed(OIS::KC_2)) animal->event(Animal::happy2);
		if(hid->isKeyPressed(OIS::KC_3)) animal->event(Animal::happy3);
		if(hid->isKeyPressed(OIS::KC_4)) animal->event(Animal::win);
		if(hid->isKeyPressed(OIS::KC_5)) animal->event(Animal::lose);
		if(hid->isKeyPressed(OIS::KC_6)) animal->event(Animal::sad);
		if(hid->isKeyPressed(OIS::KC_7)) animal->event(Animal::smiling);
		if(hid->isKeyPressed(OIS::KC_8)) animal->event(Animal::anxious);
		if(hid->isKeyPressed(OIS::KC_9)) animal->event(Animal::even);
		if(hid->isKeyPressed(OIS::KC_0)) animal->event(Animal::_none); /// reset

		if(hid->isKeyPressed(OIS::KC_T)) OgreAddOns::reloadAllTextures(gameplayBases[0]->getCosmos());
		if(hid->isKeyPressed(OIS::KC_S)) OgreAddOns::reloadAllTextures("General");
		if(hid->isKeyPressed(OIS::KC_O)) OgreAddOns::reloadAllMeshes(gameplayBases[0]->getCosmos());

		/// position animal!
		Ogre::SceneNode* animalNode = animal->getAnimalNode();
		Ogre::SceneNode* talkNode = (Ogre::SceneNode*)animalNode->getChild(0);
		/// - left
		if(hid->isKeyDown(OIS::KC_NUMPAD4)) {
			if (hid->isKeyDown(OIS::KC_LSHIFT)) {
				//animalNode->scale(scSp,scSp,scSp);
				talkNode->scale(scSp,scSp,scSp);
			} else if (hid->isKeyDown(OIS::KC_LCONTROL)) {
				animalNode->yaw(Ogre::Degree(-roSp));
			} else {
				//animalNode->translate(-trSp,0,0);
				talkNode->translate(-trSp,0,0);
			}
		}
		/// - right
		if(hid->isKeyDown(OIS::KC_NUMPAD6)) {
			if (hid->isKeyDown(OIS::KC_LSHIFT)) {
				//animalNode->scale(1./scSp,1./scSp,1./scSp);
				talkNode->scale(1./scSp,1./scSp,1./scSp);
			} else if (hid->isKeyDown(OIS::KC_LCONTROL)) {
				animalNode->yaw(Ogre::Degree(roSp));
			} else {
				//animalNode->translate(trSp,0,0);
				talkNode->translate(trSp,0,0);
			}
		}
		/// - forward
		if(hid->isKeyDown(OIS::KC_NUMPAD8)) {
			//animalNode->translate(0,0,trSp);
			talkNode->translate(0,0,trSp);
		}
		/// - backward
		if(hid->isKeyDown(OIS::KC_NUMPAD5)) {
			//animalNode->translate(0,0,-trSp);
			talkNode->translate(0,0,-trSp);
		}
		/// - up
		if(hid->isKeyDown(OIS::KC_NUMPAD7)) {
			//animalNode->translate(0,trSp,0);
			talkNode->translate(0,trSp,0);
		}
		/// - backward
		if(hid->isKeyDown(OIS::KC_NUMPAD1)) {
			//animalNode->translate(0,-trSp,0);
			talkNode->translate(0,-trSp,0);
		}

		// wireframe mode
		if(hid->isKeyPressed(OIS::KC_GRAVE)) {
			if (camera->getPolygonMode() == Ogre::PM_SOLID) {
				camera->setPolygonMode(Ogre::PM_WIREFRAME);
			} else {
				camera->setPolygonMode(Ogre::PM_SOLID);
			}
		}

#endif
		// go to next level
		if(hid->isKeyDown(OIS::KC_A) && hid->isKeyDown(OIS::KC_D) && hid->isKeyDown(OIS::KC_G)) win(gameplayBases[0], 0.4);
	}

	void Gameplay::debugUpdateStats()
	{

		Ogre::OverlayElement* element;
		const Ogre::RenderTarget::FrameStats& stats = P3DWindow::getWindow()->getStatistics();

		element= Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrLevel");
		element->setCaption("Level " + Ogre::StringConverter::toString(currentLevel+1));

#ifndef _DEPLOY
		element = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		element->setCaption("Cur " + Ogre::StringConverter::toString(stats.lastFPS));
#endif
#ifdef _DEBUG
		element = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		element->setCaption("Avr " + Ogre::StringConverter::toString(stats.avgFPS));

		//Ogre::SceneNode* animal = gameplayBases[0]->getAnimal()->getAnimalNode();
		Ogre::SceneNode* animal = (Ogre::SceneNode*)gameplayBases[0]->getAnimal()->getAnimalNode()->getChild(0);
		const Ogre::Vector3& animalPos = animal->getPosition();
		const Ogre::Quaternion& animalOr = animal->getOrientation();
		std::string animalStr = "POS:(" + FANLib::Utils::toString(animalPos.x);
		animalStr += "," + FANLib::Utils::toString(animalPos.y);
		animalStr += "," + FANLib::Utils::toString(animalPos.z);
		animalStr += " YAW:(" + FANLib::Utils::toString(animalOr.getYaw().valueDegrees()) + ")";
		animalStr += " SCA:(" + FANLib::Utils::toString(animal->getScale().x) + ")";
		element = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/Animal");
		element->setCaption("Animal: " + animalStr);
#endif
	}
}
