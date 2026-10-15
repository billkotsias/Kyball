// ----------------------------------
// P3DApp - game application launcher
// ----------------------------------
// - creates 'Ogre Root', so that it may be accessed by any other component requiring it
// - kickstarts "Introduction", then fires off "Game Menu"

#pragma once
#ifndef P3DApp_H
#define P3DApp_H

#include <string>
#include <deque>
#include "hid/HIDListener.h"
#include "OgreGpuCommandBufferFlush.h"

namespace FANLib {
	class FSLParser;
	class FSLClass;
	class Log;
}

namespace Ogre {
	class Root;
}

namespace P3D {

	class P3DScripts;

	class WindowFocusListener : public HIDListener {
	protected:
		float preMuteMasterVolume;
		Ogre::GpuCommandBufferFlush fixStuttering;
	public:
		WindowFocusListener();
		virtual void onFocusChange(bool focus);
	};

	class P3DApp {
	public:

		// file paths (filled by initPaths; '/' on Linux and Android)
		static std::string PATH_DELIMITER;
		static std::string APP_FOLDER;
		static std::string DATA_PATH;
		static std::string SOUNDS_PATH;
		static const std::string SOUNDS_EXT;
		static std::string SCRIPTS_PATH;
		static std::string PLUGINS_FILE;
		static std::string OGRE_CONFIG_FILE;
		static std::string USER_PATH;
		static std::string OGRE_LOG_FILE;
		static std::string FANLIB_LOG_FILE;
		static std::string MYGUI_LOG_FILE;
		static std::string RESOURCES_FILE;
		static const std::string UNREF_RES_DIR;
		static std::string OPTIONS_FILE;

		static void initPaths();

		// application constants
		static const char* const	WINDOW_NAME;	/// application's window name
		static const int			CYCLES_PER_SEC;	/// internal game speed

		// rendersystem ids
		static const char* const RS_BEST;
		static const char* const RS_ALTER;

		// constructor/destructor
		P3DApp();
		~P3DApp();

		// begin application
		void begin();

	private:

		bool doNotBegin;

		Ogre::Root* ogreRoot;
		P3DScripts* scripts;
		FANLib::Log* log; /// debug
		WindowFocusListener* windowFocusListener;

		// constructor helpers
		void checkCreateFolders();
		void initSound();
		void initOgre();
		void setupResources();
		void loadScripts();
		static bool stringEndsWith(const std::string&, const std::string&);

		// destructor helpers
		void releaseStaticMemory();
		void destroySound();
		void deleteSingletons();
		void deleteOgre();
		void clearScripts();
	};

}

#endif