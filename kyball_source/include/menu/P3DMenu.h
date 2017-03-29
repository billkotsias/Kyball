// ----------------------------------
// P3DMenu - provide main menu screen
// ----------------------------------
// This class may start off game immediately if 1st time run

#pragma once
#ifndef P3DMenu_H
#define P3DMenu_H

#include "OgreFrameListener.h"
#include "hid\HIDListener.h"

#include "menu\SubMainOptions.h"

#include <map>

namespace Ogre {
	class RenderWindow;
}

namespace P3D {

	class Map;

	class P3DMenu : public Ogre::FrameListener, public HIDListener {

	private:

		Ogre::RenderWindow* window;
		SubMainOptions subMainOptions;
		bool notFinished;

		Map* map;

		void rootClosed(void*);

	public:

		/// temp hack, should be solved with focused HID::KeyboardListeners! Each one injecting their own stuff
		static std::map<unsigned char, unsigned char> keyCodeTrans;
		static std::map<unsigned char, unsigned char> _keyCodeTrans();

		P3DMenu();
		~P3DMenu();

		/// FrameListener
		virtual bool frameRenderingQueued(const Ogre::FrameEvent&);

		/// start main game menu
		bool begin();

		// overrides
		virtual void onKeyEvent();
		virtual void onMouseChange();
	};

}

#endif