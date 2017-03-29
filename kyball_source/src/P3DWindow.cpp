#include "StdAfx.h"
#include "P3DWindow.h"

#include "P3DApp.h"
#include "P3DConfig.h"
#include <OgreRoot.h>

#include <core\Utils.h>
#include <string>
#include <windows.h>

namespace P3D {

	Ogre::RenderWindow* P3DWindow::window = 0;
	int P3DWindow::windowHandle = 0;
	double P3DWindow::pixelRatio = 1.;

	Ogre::RenderWindow* P3DWindow::create() {

		/// x, y, full-screen, bpp, FSAA
		int windowX;
		int windowY;
		//windowX = 480; /// 16:9 = 1800 / 4:3 = 1352 (for y = 1012) / 5:4 = 1265
		//windowY = 320;
		windowX = P3DConfig::WIDTH;
		windowY = P3DConfig::HEIGHT;
		std::string colourDepth = FANLib::Utils::toString(P3DConfig::DEPTH);	/// "16", "32" or "" (Ogre default)
#ifdef _DEBUG
		bool fullScreen = false;
#else
		bool fullScreen = true;
#endif

		Ogre::NameValuePairList list;
		list["FSAA"] = FANLib::Utils::toString(P3DConfig::FSAA);	/// "0", "2", "4", "8"...
		list["vsync"] = "true";										/// <vsync> always <on> (except for debug purposes)
		list["Floating-point mode"] = "Consistent";					/// just in case
		list["colourDepth"] = colourDepth;
		list["displayFrequency"] = "60";

		/// create corresponding window
		setWindow( Ogre::Root::getSingletonPtr()->createRenderWindow(P3DApp::WINDOW_NAME, windowX, windowY, fullScreen, &list) );

		return window;
	}

	void P3DWindow::setWindow(Ogre::RenderWindow* win) {
		window = win;
		window->setDeactivateOnFocusChange(false);
		window->getCustomAttribute("WINDOW", &windowHandle); /// get window <handle>
		if (window->isFullScreen()) {
			pixelRatio = P3DConfig::MONITOR_ASPECT_RATIO * window->getHeight() / window->getWidth();
		} else {
			pixelRatio = 1.;
		}
	}

	//Ogre::RenderWindow* P3DWindow::recreate() {
	//	/// <TODO> : UNIMPLEMENTED
	//	return 0;
	//}

	int P3DWindow::getWindowHandle() {
		return windowHandle;
	}

	bool P3DWindow::focused() {
		return ((int)GetForegroundWindow() == (int)windowHandle);
	}

	Ogre::Vector2 P3DWindow::getSize() {
		static Ogre::Vector2 size(window->getWidth(), window->getHeight());
		return size;
	}

	double P3DWindow::getWindowRatio() {
		static double ratio = (double)window->getWidth() / window->getHeight();
		return ratio;
	}
}