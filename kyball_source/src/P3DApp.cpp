#include "StdAfx.h"
#include "P3DApp.h"

#include <OgreRoot.h>
#include <OgreException.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreRenderSystem.h>
#include <OgreStringVector.h>
#include <windows.h>
#include <vector>

#include "P3DWindow.h"
#include "P3DScripts.h"
#include "P3DIntro.h"
#include "menu\P3DMenu.h"

// singletons
#include "SceneBaseCreator.h"
#include "visual\VisualBoss.h"
#include "hid\HIDBoss.h"
#include "tween\TPlayer.h"
#include "sound\SoundBoss.h"
#include "sound\Sound.h"

#include <core\Log.h>
#include <fsl\FSLParser.h>
#include <fsl\FSLClass.h>

#include <OgreD3D9RenderSystem.h>

#include <time.h>
#include <core\utils.h>

// temp!
#include <P3DExtro.h>

namespace P3D {

	const char* const	P3DApp::WINDOW_NAME		= "Kyball";
	const int			P3DApp::CYCLES_PER_SEC	= 60; /// NOTE : Define gameplay values <relatively> to this variable, as it may increase!

	const char* const P3DApp::RS_BEST = "Direct3D9";	/// Direct3D9
	const char* const P3DApp::RS_ALTER = "OpenGL";		/// OpenGL

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	const std::string P3DApp::PATH_DELIMITER	= "\\"; /// < \ >
	const std::string P3DApp::USER_PATH			= std::getenv("APPDATA") + P3DApp::PATH_DELIMITER;
#endif
#ifdef _DEMO
	const std::string P3DApp::APP_FOLDER		= USER_PATH + "KYBALL_DEMO" + P3DApp::PATH_DELIMITER;
#else
	const std::string P3DApp::APP_FOLDER		= USER_PATH + "KYBALL" + P3DApp::PATH_DELIMITER;
#endif
#if defined(_DEPLOY) || !defined(_DEBUG)
//#if defined(_DEPLOY)
	const std::string P3DApp::DATA_PATH			= "P3DData" + P3DApp::PATH_DELIMITER;
#else
	const std::string P3DApp::DATA_PATH			= "..\\P3DData" + P3DApp::PATH_DELIMITER;
#endif
	const std::string P3DApp::PLUGINS_FILE		= "P3DData\\plugins.cfg";
	const std::string P3DApp::OGRE_CONFIG_FILE	= APP_FOLDER + "ogre.cfg";
	const std::string P3DApp::OGRE_LOG_FILE		= APP_FOLDER + "ogre.log";
	const std::string P3DApp::FANLIB_LOG_FILE	= APP_FOLDER + "fanlib.log";
	const std::string P3DApp::MYGUI_LOG_FILE	= APP_FOLDER + "mygui.log";
	const std::string P3DApp::RESOURCES_FILE	= DATA_PATH + "resources.cfg";
	const std::string P3DApp::SCRIPTS_PATH		= DATA_PATH + "scripts" + P3DApp::PATH_DELIMITER;
	const std::string P3DApp::SOUNDS_PATH		= DATA_PATH + "snd" + P3DApp::PATH_DELIMITER;
#if defined(_DEPLOY) || !defined(_DEBUG)
	const std::string P3DApp::SOUNDS_EXT		= ".ogg";
#else
	const std::string P3DApp::SOUNDS_EXT		= ".wav";
#endif
	const std::string P3DApp::UNREF_RES_DIR		= "unref"; /// directory paths ending like this are assumed to have script-unreferenced resources (must be declared manually)
	const std::string P3DApp::OPTIONS_FILE		= APP_FOLDER + "options2.txt";

	P3DApp::P3DApp() {
		doNotBegin = false;

		srand( time(0) );	/// initialize random seed

		checkCreateFolders();
		loadScripts();		/// parse FSL scripts : must precede all other initializations!
		initOgre();			/// Ogre initialization
		initSound();		/// OpenAL initialization - MUST COME AFTER Ogre INIT
	}

	P3DApp::~P3DApp() {
		releaseStaticMemory();

		destroySound();
		deleteSingletons();
		deleteOgre();
		clearScripts();		/// remove scripts last : exiting settings may be needed
	}

	// begin application
	void P3DApp::begin() {
		if (doNotBegin) return;

		// time test - here for "convenience"
		//clock_t time = clock();
		//for (int i = 0; i < 1000000; ++i) {
		//	int t = (int)GetForegroundWindow() + 1;
		//}
		//time = clock() - time;
		//std::cout << "Time taken" << FANLib::Utils::toString( (float)(time) / CLOCKS_PER_SEC ) << "\n";

		/// begin introduction
		/// <= ignore any errors that might occur; after all, this is only the introduction
		try {
			{
				P3DIntro intro;
				intro.begin();
			}
		} catch(...) {}

		/// go into game menu
		try
		{
			bool showEndCredits;
			do
			{
				{
					P3DMenu menu;
					showEndCredits = menu.begin();
				}

				if ( showEndCredits )
				{
#ifdef _DEMO
					P3DIntro intro;
					intro.begin("DEMO", 1., 120.);
#else
					P3DExtro extro;
					extro.begin();
#endif
				}

			} while (showEndCredits);

		} catch( Ogre::Exception& e ) {
			MessageBox( NULL, e.getFullDescription().c_str(),
						"An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		}
	}



	// constructor helping functions

	/* Init Ogre */;
	void P3DApp::initOgre() {

		ogreRoot = OGRE_NEW Ogre::Root(PLUGINS_FILE, OGRE_CONFIG_FILE, OGRE_LOG_FILE);

		// Show default Ogre window :
		//if (!ogreRoot->showConfigDialog()) {
		//	doNotBegin = true;
		//	return;
		//}
		//ogreRoot->initialise(true);

		//if (!ogreRoot->restoreConfig()) {
		if (true) {

			/// restoring last gfx settings failed : attempt to manually configure render system
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
				//ogreRoot->saveConfig();					/// if all went fine till this point, save this configuration for future reference
			}

		} else {
			ogreRoot->initialise(false);
		}

		/// create application window; graphics <device> is now built
		Ogre::RenderWindow* window = P3DWindow::create();
		HIDBoss::windowHandle = P3DWindow::getWindowHandle();

		/// initialize Ogre SceneBaseCreator
		SceneBaseCreator::getSingletonPtr()->init(window);
		///SceneBaseCreator::getSingletonPtr()->init(ogreRoot->getAutoCreatedWindow());

		/// now that we have a window, we have a "rendering-context" for our <Ogre resources> and we can initialize them
		setupResources();

		/// get initialized rendersystem - redundant because Ogre does what it likes
		//Ogre::RenderSystem* renderSystem = ogreRoot->getRenderSystem();
		/// - set <D3D> specific options
		//if (renderSystem->getName().find("Direct3D") != std::string::npos) {
		//	Ogre::D3D9RenderSystem* d3dSystem = (Ogre::D3D9RenderSystem*)renderSystem;
		//	renderSystem->_setAlphaRejectSettings(Ogre::CMPF_GREATER, 128, true);

		//	HRESULT hres;
		//	/// - get <vendor>
		//	const Ogre::RenderSystemCapabilities* caps = d3dSystem->getCapabilities();
		//	switch (caps->getVendor()) {

		//		case Ogre::GPU_NVIDIA:
		//			/// - set <transparency antialiasing>
		//			hres = d3dSystem->getDevice()->SetRenderState(D3DRS_ADAPTIVETESS_Y, (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C'));
		//			break;

		//		case Ogre::GPU_ATI:
		//			/// - set <transparency antialiasing>
		//			hres = d3dSystem->getDevice()->SetRenderState(D3DRS_POINTSIZE, MAKEFOURCC('A','2','M','1'));
		//			break;
		//	}
		//}
	}

	/* is string's end equal to another string? */;
	bool P3DApp::stringEndsWith(const std::string& source, const std::string& end) {
		if (source.substr(source.length() - end.length()) == end) return true;
		return false;
	}

	/* Setup Ogre resources */;
	void P3DApp::setupResources() {

        /// - load "resources" file
		Ogre::ResourceGroupManager* resMan =  Ogre::ResourceGroupManager::getSingletonPtr();
		Ogre::ConfigFile cf;
        cf.load(RESOURCES_FILE);

        /// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

		std::string secName, typeName, archName;
		std::map<std::string, bool> unrefResourceGroups; /// separate in here dirs containing script-unreferenced resources

		while (seci.hasMoreElements()) {

            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); ++i) {
                typeName = i->first;
				archName = DATA_PATH + i->second; /// NOTE : "P3DData\" automatically added to all resource filenames

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                ResourceGroupManager::getSingleton().addResourceLocation(
                    String(macBundlePath() + "/" + archName), typeName, secName);
#else
				resMan->addResourceLocation(archName, typeName, secName); /// e.g "..\P3DData\gfx", "Filesystem", "GAMEPLAY"
				if (stringEndsWith(archName, UNREF_RES_DIR)) unrefResourceGroups[secName] = true;
				//std::cout << archName << " " << typeName << " " << secName << "\n";
#endif
            }
		}

		/// declare all resources of resource-group directories names "...unref", cause they have no script-reference to automatically declare them! Ogre shortcoming!
		for (std::map<std::string, bool>::iterator it = unrefResourceGroups.begin(); it != unrefResourceGroups.end(); ++it) {
			const std::string& groupName = it->first;
			Ogre::FileInfoListPtr fileInfoList = resMan->listResourceFileInfo(groupName);
			for (unsigned int i = 0; i < fileInfoList->size(); ++i) {
				const Ogre::FileInfo& fileInfo = fileInfoList->at(i);
				//std::cout << "unref f**k:" << groupName << " " << fileInfo.archive->getName() << "\n";
				if (stringEndsWith(fileInfo.archive->getName(), UNREF_RES_DIR)) {
					resMan->declareResource(fileInfo.basename, "Texture", groupName); /// they must all be <Texture>s !
				}
			}
		}

		resMan->initialiseAllResourceGroups();
	}

	void P3DApp::initSound() {
		SoundBoss::init(P3DApp::SOUNDS_PATH, SOUNDS_EXT);
		Sound::readScript();
		/// audio muter
		windowFocusListener = new WindowFocusListener(); /// automatically listen to focus changes
	}

	WindowFocusListener::WindowFocusListener() : HIDListener(false, false, true) {
		fixStuttering.start();
	}
	void WindowFocusListener::onFocusChange(bool focus) {
		if (focus) {
			SoundBoss::getSingletonPtr()->setMasterVolume(preMuteMasterVolume);
			//fixStuttering.start(); // best solution! Just don't restore f***ing frame rate fix!
		} else {
			preMuteMasterVolume = SoundBoss::getSingletonPtr()->getMasterVolume();
			SoundBoss::getSingletonPtr()->setMasterVolume(0);
			fixStuttering.stop(); // best solution!
		}
	}

	void P3DApp::destroySound() {
		delete windowFocusListener;
		windowFocusListener = 0;
		Sound::destroyAll();
		SoundBoss::destroy();
	}

	/* Release memory held statically */;
	void P3DApp::releaseStaticMemory() {
		//ShadowManager::releaseMemory(); not withstanding
	}

	/* Delete all application singletons */;
	void P3DApp::deleteSingletons() {
		HIDBoss::destroy();
		VisualBoss::destroy();
		//TPlayer::destroy(); // crash due to TPlaylists trying to reach final condition. Stupid that.
		SceneBaseCreator::destroy();
	}

	/* End Ogre */;
	void P3DApp::deleteOgre() {
		delete ogreRoot;
	}

	/* Parse FSL scripts */;
	void P3DApp::loadScripts() {
#ifndef _DEPLOY
		log = new FANLib::Log(FANLIB_LOG_FILE.c_str(), false, false, true, true); /// without console output
		//log = new FANLib::Log(FANLIB_LOG_FILE.c_str(), false, true, true, true); /// with console output enabled
#endif
		P3DScripts::load();
	}

	void P3DApp::clearScripts() {
		P3DScripts::unload();
#ifdef _DEBUG
		delete log;
#endif
	}

	void P3DApp::checkCreateFolders() {
		CreateDirectoryA(P3DApp::APP_FOLDER.c_str(), 0);
	}

}
