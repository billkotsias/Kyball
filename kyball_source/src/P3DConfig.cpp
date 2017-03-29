#include "StdAfx.h"
#include "P3DConfig.h"

#include "P3DApp.h"
#include "P3DWindow.h"
#include <SceneBaseCreator.h>
#include "hid\HIDBoss.h"
#include "menu\SubConfig.h"

#include <core\Utils.h>
#include <math\Math.h>
#include <iostream>
#include <fstream>

namespace P3D {

	unsigned int P3DConfig::WIDTH = 800;
	unsigned int P3DConfig::HEIGHT = 600;
	unsigned int P3DConfig::DEPTH = 32;
	unsigned int P3DConfig::FSAA = 0;
	GameQuality::Option P3DConfig::gameQuality = GameQuality::SUPA_DUPA;

	double P3DConfig::MONITOR_ASPECT_RATIO = 4./3.;

	const std::string P3DConfig::RESOURCES_FILE	= P3DApp::DATA_PATH + "configRes.cfg";

	P3DConfig::~P3DConfig() {
		delete subConfig;

		SceneBaseCreator::destroy();
		HIDBoss::destroy();
		delete ogreRoot;
	}

	bool P3DConfig::begin() {

		doNotBegin = false;
		notFinished = true;
		subConfig = 0;
		ogreRoot = 0;

		// read parameters
		/// "guess" monitor aspect ratio and max resolution by current desktop screen-mode
		DEVMODEA modeInfo;
		modeInfo.dmSize = sizeof(DEVMODEA);
		modeInfo.dmDriverExtra = NULL;
		EnumDisplaySettingsExA(0, ENUM_CURRENT_SETTINGS, &modeInfo, 0);
		currentMode = modeInfo;

		MONITOR_ASPECT_RATIO = (double)currentMode.dmPelsWidth / (double)currentMode.dmPelsHeight; /// square-pixel assumed, or the user is plain crazy

		// before anything else, check if options file exists with settled settings (PUN)!
		try {
			//bool showGfx;
			do {
				std::fstream file( P3DApp::OPTIONS_FILE.c_str(), std::fstream::binary|std::fstream::in);
				if (file.fail() || !file.is_open()) {
					// 1st time run probably, use MAX settings as Ann suggested!
					gameQuality = GameQuality::SUPA_DUPA;
					break;
				}

				file.read(reinterpret_cast<char*>(&gameQuality), sizeof(gameQuality));

				//file.read(reinterpret_cast<char*>(&showGfx), sizeof(showGfx));
				//if (showGfx) {
				//	file.close();
				//	break; /// move on to show menu
				//}
				//file.read(reinterpret_cast<char*>(&WIDTH), sizeof(WIDTH));
				//file.read(reinterpret_cast<char*>(&HEIGHT), sizeof(HEIGHT));
				//file.read(reinterpret_cast<char*>(&DEPTH), sizeof(DEPTH));
				//file.read(reinterpret_cast<char*>(&FSAA), sizeof(FSAA));

				file.close();

			} while(false);
		} catch(...) {};

		// enumerate all suitable screen modes
		int modeNum = -1;
		while (EnumDisplaySettingsExA(0, ++modeNum, &modeInfo, 0)) {
			if (modeInfo.dmBitsPerPel < 16) continue;
			modes.push_back( modeInfo );
		}

		// show config menu
		ogreRoot = OGRE_NEW Ogre::Root(P3DApp::PLUGINS_FILE, P3DApp::OGRE_CONFIG_FILE, P3DApp::OGRE_LOG_FILE);

		Ogre::RenderSystem* renderSystem = 0;
		Ogre::RenderSystemList* renderSystems = ogreRoot->getAvailableRenderers();

		// TODO : favourable/fallback rendering systems should be defined externally in a script
		for (unsigned int i = 0; i < renderSystems->size(); ++i) {
			Ogre::RenderSystem* availableSystem = renderSystems->at(i);
			/// - <almost-worst> case : no DirectX 9 or OpenGL found; get whatever system is <first> available
			if (i == 0) renderSystem = availableSystem;
			/// - <best> case : DirectX 9 found
			if (availableSystem->getName().find(P3DApp::RS_BEST) != std::string::npos) { /// default best : Direct3D9
				renderSystem = availableSystem;
				break;
			}
			/// - <medium> case : OpenGL found (is this not Windows?)
			if (availableSystem->getName().find(P3DApp::RS_ALTER) != std::string::npos)
				renderSystem = availableSystem;
		}
		if (!renderSystem) {
			/// <worst> case : no systems available
			ogreRoot->showConfigDialog(); /// this won't solve anything, but may help in defining <point-of-crash>
			renderSystem = ogreRoot->getRenderSystem();
		}
		if (renderSystem) {
			ogreRoot->setRenderSystem(renderSystem);	/// set hopefully-fine renderer
			ogreRoot->initialise(false);				/// ... fingers crossed ...
		}

		// get available FSAA settings
		Ogre::ConfigOptionMap settings = renderSystem->getConfigOptions();
		for (Ogre::ConfigOptionMap::iterator it = settings.begin(); it != settings.end(); ++it) {
			Ogre::_ConfigOption opt = it->second;
			if (opt.name != "Anti aliasing") continue; /// <NOTE> : "FSAA" from Ogre 1.7 onwards!!!
			const Ogre::StringVector& values = opt.possibleValues;
			for (unsigned int i = 0; i < values.size(); ++i) {
				const std::string& value = values.at(i);
				if (Ogre::StringUtil::startsWith(value, "Level", false)) {
					int v;
					FANLib::Utils::fromString(v, value.substr(5));
					fsaa.push_back(v);
#ifdef _DEBUG
					std::cout << "FSAA Opt:" << v << "\n";
#endif
				}
			}
		}

		setMode( gameQuality );
		return doNotBegin;

		//
		//

		/// graphics <device> is now built : create now application window
		/// - use the minimum of graphics settings
		int windowX = 800;
		int windowY = 600;
		std::string colourDepth = "";	/// "16", "32" or "" (Ogre default)
		Ogre::NameValuePairList list;
		list["vsync"] = "true";
		list["Floating-point mode"] = "Consistent";	/// just in case
		bool fullScreen = true; // This is GH man! (was: This is the config window man!)

		/// create corresponding window
		int windowHandle;
		Ogre::RenderWindow* window = Ogre::Root::getSingletonPtr()->createRenderWindow(P3DApp::WINDOW_NAME, windowX, windowY, fullScreen, &list);
		window->setDeactivateOnFocusChange(true);
		window->getCustomAttribute("WINDOW", &windowHandle); /// get window <handle>
		HIDBoss::windowHandle = windowHandle;

		/// initialize Ogre SceneBaseCreator
		P3DWindow::setWindow(window);
		SceneBaseCreator::getSingletonPtr()->init(window);

		/// now that we have a window, we have a "rendering-context" for our <Ogre resources> and we can initialize them
		Ogre::ResourceGroupManager* resMan =  Ogre::ResourceGroupManager::getSingletonPtr();
		Ogre::ConfigFile cf;
        cf.load(RESOURCES_FILE);

        /// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		std::string secName, typeName, archName;
		while (seci.hasMoreElements()) {

            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); ++i) {
                typeName = i->first;
				archName = P3DApp::DATA_PATH + i->second; /// NOTE : "P3DData\" automatically added to all resource filenames

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                ResourceGroupManager::getSingleton().addResourceLocation(
                    String(macBundlePath() + "/" + archName), typeName, secName);
#else
				resMan->addResourceLocation(archName, typeName, secName); /// e.g "..\P3DData\gfx", "Filesystem", "GAMEPLAY"
#endif
            }
		}

		resMan->initialiseAllResourceGroups();

		// create and show menu
		SceneBaseCreator::getSingletonPtr()->createSceneBase();

		subConfig = new SubConfig();
		subConfig->setCallBack(this, &P3DConfig::subConfigClosed);
		subConfig->open();

		{
			P3DConfigHelper helper(this); /// sends mouse clicks to MyGUI

			MyGUI::Gui::getInstancePtr()->setVisiblePointer(true);
			ogreRoot->addFrameListener(this);
			{	/// run a frame
				Ogre::FrameEvent runOnce;
				runOnce.timeSinceLastFrame = 1./P3DApp::CYCLES_PER_SEC;
				frameRenderingQueued(runOnce);
			}
			ogreRoot->startRendering();
			ogreRoot->removeFrameListener(this);
			MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);
		}

		return doNotBegin;
	}

	void P3DConfig::subConfigClosed(void*)
	{
		setMode( subConfig->selected );
	}

	void P3DConfig::setMode(GameQuality::Option option)
	{
		DEVMODEA mode;
		switch ( option ) {
			case GameQuality::LOW:
				mode = getMinMode();
				FSAA = getMinFSAA();
				break;
			case GameQuality::MEDIUM:
				mode = getMedMode();
				FSAA = getMedFSAA();
				break;
			case GameQuality::HIGH:
			case GameQuality::ADVANCED:
			case GameQuality::SUPA_DUPA:
				mode = getMaxMode();
				FSAA = getMedFSAA(); /// don't touch <max> !!!
				break;
		}
		WIDTH = mode.dmPelsWidth;
		HEIGHT = mode.dmPelsHeight;
		DEPTH = mode.dmBitsPerPel;

		std::cout << "MODE SET:" << WIDTH << "*" << HEIGHT << "*" << DEPTH << "*" << FSAA <<"\n";
		// all required parameters are set now!

		// VIDEO PRODUCTION HACK
		//WIDTH = 1280;
		//HEIGHT = 720;
		//DEPTH = 32;
		//FSAA = 4;

		notFinished = false;
	}

	bool P3DConfig::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		if (P3DWindow::getWindow()->isClosed()) {
			doNotBegin = true;
			return false; /// app was quit externally!
		}

		/// tweener
		TPlayer::getSingletonPtr()->run(evt.timeSinceLastFrame);

		/// input
		static int x,y;
		HIDBoss::getSingletonPtr()->captureDevices();
		HIDBoss::getSingletonPtr()->getMousePosition(x, y);
		MyGUI::InputManager::getInstancePtr()->injectMouseMove(x, y, 0);

		return notFinished;
	}


	unsigned int P3DConfig::getMaxFSAA() {
		if (fsaa.empty()) return 0;

		unsigned int max = 0;
		for (unsigned int i = 0; i < fsaa.size(); ++i) {
			const unsigned int& v = fsaa.at(i);
			if (max < v) max = v;
		}
		return max;
	}
	unsigned int P3DConfig::getMinFSAA() {
		if (fsaa.empty()) return 0;

		unsigned int min = 0xFFFFFFFF;
		for (unsigned int i = 0; i < fsaa.size(); ++i) {
			const unsigned int& v = fsaa.at(i);
			if (min > v) min = v;
		}
		return min;
	}
	unsigned int P3DConfig::getMedFSAA() {
		if (fsaa.empty()) return 0;

		unsigned int max = getMaxFSAA();
		unsigned int min = getMinFSAA();
		unsigned int _med = (max + min) >> 1;

		unsigned int med;
		unsigned int diff = 0xFFFFFFFF;

		for (unsigned int i = 0; i < fsaa.size(); ++i) {
			const unsigned int& v = fsaa.at(i);
			unsigned int _diff = abs((int)(_med - v));
			if (_diff < diff) {
				diff = _diff;
				med = v;
			}
		}
		return med;
	}

	const DEVMODEA& P3DConfig::getMaxMode() {
		return currentMode;
	}
	const DEVMODEA& P3DConfig::getMinMode() {

		static const unsigned int X_MIN = 800;
		static const unsigned int Y_MIN = 600;

		const DEVMODEA* min = getNearestMode(X_MIN, Y_MIN, 16);
		if (!min) min = getNearestMode(X_MIN, Y_MIN, 32);
		return *min;
	}
	const DEVMODEA& P3DConfig::getMedMode()
	{
		//unsigned int Y_MIN = (currentMode.dmPelsHeight + 600) / 2;
		unsigned int Y_MIN = 800;

		const DEVMODEA* min = getNearestMode(MONITOR_ASPECT_RATIO*Y_MIN, Y_MIN, 32);
		if (!min) min = getNearestMode(MONITOR_ASPECT_RATIO*Y_MIN, Y_MIN, 16);
		return *min;
	}

	const DEVMODEA* P3DConfig::getNearestMode(unsigned int x, unsigned int y, unsigned int bpp) {

		const DEVMODEA* nearestMode = 0;
		double diff = Infinite;

		for (unsigned int i = 0; i < modes.size(); ++i) {
			const DEVMODEA& mode = modes.at(i);
			if (mode.dmBitsPerPel != bpp) continue;

			double diffx = mode.dmPelsWidth - x;
			double diffy = mode.dmPelsHeight - y;
			double _diff = diffx * diffx + diffy * diffy;
			if (_diff < diff) {
				diff = _diff;
				nearestMode = &mode;
			}
		}
		return nearestMode;
	}

	//

	void P3DConfigHelper::onKeyEvent() {
		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isKeyReleased(OIS::KC_ESCAPE)) {
			cfg->notFinished = false;
			cfg->doNotBegin = true;
		}
	}

	void P3DConfigHelper::onMouseChange() {
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