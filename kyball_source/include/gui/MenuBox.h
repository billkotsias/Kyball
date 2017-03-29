/////////////
// MenuBox
/////////////

#pragma once
#ifndef MenuBox_H
#define MenuBox_H

#include "OverlayBox.h"
#include <hid\HIDListener.h>

#include <core\MCBSystem.h>
#include <string>

namespace P3D {

	class MenuBox : public OverlayBox, public HIDListener, public FANLib::MCBSystem {

	protected:

		bool checkBelowMouse();
		bool belowMouse;

		/// mouse over...
		static const Ogre::Vector2 OVER_SIZE;	/// multiplied with 'defaultSize'
		static const double OVER_TIME;
		static const int CLICK_DIST;

	public:

		enum CallBacks {
			_none,
			OPENING,
			CLICKED,
			CLOSED,
			_last
		};

		MenuBox(const std::string& name, const std::string& clonePrefix = "");
		virtual ~MenuBox() {};

		static const double APPEAR_TIME;
		static const double DISAPPEAR_TIME;

		Ogre::Vector2 defaultSize;	/// } set these before 'open', as well as position
		double delay;				/// }

		virtual void open();
		virtual void close();

		/// overrides
		virtual void playlistFinished(TPlaylist*);
		virtual void onMouseChange();
		virtual void onMouseMove();
	};
}

#endif