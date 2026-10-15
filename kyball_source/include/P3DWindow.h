// -----------------------------------------
// P3DWindow - application window properties
// -----------------------------------------

#pragma once
#ifndef P3DWindow_H
#define P3DWindow_H

#include "GameQuality.h"

namespace Ogre {
	class RenderWindow;
}

namespace P3D {

	class P3DWindow {
	private:

		static Ogre::RenderWindow* window;
		static int windowHandle;

		static double pixelRatio;

	public:

		static void setWindow(Ogre::RenderWindow*);

		static int getWindowHandle();

		static Ogre::RenderWindow* create();
		static Ogre::RenderWindow* getWindow() { return window; }
		//static Ogre::RenderWindow* recreate();

		static bool focused();
		// The window manager asked us to close. Do not destroy the GL window
		// inside the frame that sees the event; the swap that follows would
		// touch a dead drawable and kill the process.
		static void requestClose();
		static bool closeRequested();

		static Ogre::Vector2 getSize();
		static double getPixelRatio() { return pixelRatio; };
		static double getWindowRatio();
	};

}

#endif