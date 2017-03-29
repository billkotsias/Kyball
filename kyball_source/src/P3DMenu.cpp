#include "StdAfx.h"
#include "menu\P3DMenu.h"

#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "P3DApp.h"
#include "P3DWindow.h"
#include "game\Game.h"
#include "map\Map.h"

#include "menu\SubMain.h"
#include "menu\PlayerBoss.h"
#include "menu\Player.h"

#include "hid\HIDBoss.h"
#include "tween\TPlayer.h"
#include "core\Utils.h"

#include "P3DExtro.h"

namespace P3D {

	std::map<unsigned char, unsigned char> P3DMenu::keyCodeTrans = _keyCodeTrans();
	std::map<unsigned char, unsigned char> P3DMenu::_keyCodeTrans() {
		std::map<unsigned char, unsigned char> trans;
		trans[0x02] = '1';
		trans[0x03] = '2';
		trans[0x04] = '3';
		trans[0x05] = '4';
		trans[0x06] = '5';
		trans[0x07] = '6';
		trans[0x08] = '7';
		trans[0x09] = '8';
		trans[0x0A] = '9';
		trans[0x0B] = '0';
		trans[0x4F] = '1';
		trans[0x50] = '2';
		trans[0x51] = '3';
		trans[0x4B] = '4';
		trans[0x4C] = '5';
		trans[0x4D] = '6';
		trans[0x47] = '7';
		trans[0x48] = '8';
		trans[0x49] = '9';
		trans[0x52] = '0';
		trans[0x10] = 'Q';
		trans[0x11] = 'W';
		trans[0x12] = 'E';
		trans[0x13] = 'R';
		trans[0x14] = 'T';
		trans[0x15] = 'Y';
		trans[0x16] = 'U';
		trans[0x17] = 'I';
		trans[0x18] = 'O';
		trans[0x19] = 'P';
		trans[0x1E] = 'A';
		trans[0x1F] = 'S';
		trans[0x20] = 'D';
		trans[0x21] = 'F';
		trans[0x22] = 'G';
		trans[0x23] = 'H';
		trans[0x24] = 'J';
		trans[0x25] = 'K';
		trans[0x26] = 'L';
		trans[0x2C] = 'Z';
		trans[0x2D] = 'X';
		trans[0x2E] = 'C';
		trans[0x2F] = 'V';
		trans[0x30] = 'B';
		trans[0x31] = 'N';
		trans[0x32] = 'M';
		return trans;
	};

	P3DMenu::P3DMenu() {
		map = new Map();
	}

	P3DMenu::~P3DMenu() {
		delete map;
	}

	bool P3DMenu::begin() {

		/// menu and game init
		Game game;
		game.map = map;

		PlayerBoss playerBoss(P3DApp::APP_FOLDER);

		/// Ogre init
		Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
		Ogre::ResourceGroupManager* resManager = Ogre::ResourceGroupManager::getSingletonPtr();
		window = P3DWindow::getWindow();

		bool appQuit;
		bool showEndCredits = false;

		do {
			/* menu construction */;
			/// - create Scene base through <Map>
			map->init();
			map->showDemo();

			/// - show root submenu
			SubMain* subMain = new SubMain(&playerBoss, &subMainOptions); ///...pass "subMainOptions" to root submenu
			subMain->setCallBack(this, &P3DMenu::rootClosed);
			subMain->open();

			Ogre::Vector2 screenCenter = P3DWindow::getSize() / 2.;
			HIDBoss::getSingletonPtr()->setMousePosition(screenCenter.x, screenCenter.y);
			MyGUI::Gui::getInstancePtr()->setVisiblePointer(true);

			/// - start rendering loop : will end if root submenu is closed, or application window is closed
			ogreRoot->addFrameListener(this);
			acceptKeyEvents(true);
			acceptMouseEvents(true);
			notFinished = true;

			{
				/// run one frame before rendering starts
				Ogre::FrameEvent runOnce;
				runOnce.timeSinceLastFrame = 1./P3DApp::CYCLES_PER_SEC;
				frameRenderingQueued(runOnce);
			}
			ogreRoot->startRendering();

			// rendering loop finished -> main submenu has returned with user choices (subMainOptions != 0)
			HIDBoss::getSingletonPtr()->clippingRectEnabled = false;
			acceptKeyEvents(false);
			acceptMouseEvents(false);
			ogreRoot->removeFrameListener(this);
			//MyGUI::Gui::getInstancePtr()->setVisiblePointer(false); // let new "state" decide if it wants it or not?
			map->hideDemo();

			/// - destroy root submenu
			delete subMain;

			/// - process user choices
			switch (subMainOptions.finalChoice) {
				default:
				case SubMainOptions::QUIT : /// proceed to end menu loop
					appQuit = true;
					break;

				case SubMainOptions::SHOW_END_CREDITS :
					appQuit = showEndCredits = true;
					break;

				case SubMainOptions::ADVENTURE :
					appQuit = game.adventure(subMainOptions.player);
					showEndCredits = game.showEndCredits;
					break;

				case SubMainOptions::VERSUS :
					appQuit = game.versus(subMainOptions.player);
					break;
				case SubMainOptions::SURVIVAL :
					appQuit = game.survival(subMainOptions.player);
					break;
				//default:
				//	OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
				//				std::string("'SubMain' returned invalid final choice : ") + FANLib::Utils::toString<int>(subMainOptions.finalChoice),
				//				"P3DMenu");
			}

			P3DExtro::ShowSuperEnding = (playerBoss.getActive()) ? playerBoss.getActive()->is100Percent() : false;

		} while(!appQuit); /// main menu loop doesn't end until : finalChoice = QUIT or there was a quit request from "game"

		return showEndCredits;
	}

	// FrameListener
	// - frame queued
	bool P3DMenu::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		if (window->isClosed()) return false; /// check if app was externally quit

		int frames = P3DApp::CYCLES_PER_SEC * evt.timeSinceLastFrame; /// loop will run at least once
		static const double frameTime = 1./P3DApp::CYCLES_PER_SEC;
		do {
			/// tweener
			TPlayer::getSingletonPtr()->run(frameTime);

			/// input
			static int x,y;
			HIDBoss::getSingletonPtr()->captureDevices();
			HIDBoss::getSingletonPtr()->getMousePosition(x, y);
			MyGUI::InputManager::getInstancePtr()->injectMouseMove(x, y, 0);
			map->runDemoFrame(frameTime);

		} while (--frames > 0);

		/// if root submenu has closed (which we know through the 'SubmenuListener' interface) -> return false, else true (continue menu loop)
		return notFinished;
	}

	void P3DMenu::rootClosed(void *root) {
		notFinished = false;
	}

	void P3DMenu::onKeyEvent() {

		HIDBoss* hid = HIDBoss::getSingletonPtr();

		/// inject key changes to MyGUI
		for (unsigned int i = 0; i < 256; ++i) {
			if (hid->isKeyPressed(i)) {
				MyGUI::InputManager::getInstancePtr()->injectKeyPress((MyGUI::KeyCode::Enum)i, keyCodeTrans[i]);
			}
		}
		for (unsigned int i = 0; i < 256; ++i) {
			if (hid->isKeyReleased(i)) {
				MyGUI::InputManager::getInstancePtr()->injectKeyRelease((MyGUI::KeyCode::Enum)i);
			}
		}

		/// Escape?
		if (hid->isKeyPressed(OIS::KC_ESCAPE)) {
			/// <TODO> : Open "Are you sure" window!!!
			subMainOptions.finalChoice = SubMainOptions::QUIT;
			notFinished = false;
		}
	}

	void P3DMenu::onMouseChange() {
		static int x,y;

		HIDBoss* hid = HIDBoss::getSingletonPtr();
		hid->getMousePosition(x, y);

		MyGUI::InputManager* min = MyGUI::InputManager::getInstancePtr();

		if (hid->isButtonPressed(HIDBoss::MOUSE_LEFT)) {
			min->injectMousePress(x, y, MyGUI::MouseButton::Left);
		}
		if (hid->isButtonReleased(HIDBoss::MOUSE_LEFT)) {
			min->injectMouseRelease(x, y, MyGUI::MouseButton::Left);
		}
	}
}