//////////////
// HIDListener - Listens to 'HIDBoss' events
//////////////

#pragma once
#ifndef HIDListener_H
#define HIDListener_H

namespace P3D {

	class HIDMouseEvent;

	class HIDListener {

	public:

		HIDListener(bool key = false, bool mouse = false, bool focus = false,
					bool joy = false, bool clickQuery = false, bool moveQuery = false);
		virtual ~HIDListener();

		/// local parameters for Mouse Click/Double-Click
		double clickMaxDistance; /// maximum distance between mouse down and up to permit for clicks/double clicks
		//double 

		/// key events
		virtual void onKeyEvent() {};	/// listener can now query 'HIDBoss' for details (eg. which key or if any key was pressed) - <continuously called !!!>

		/// basic mouse events
		virtual void onMouseDown()		{}; /// continuously triggered if a button is pressed
		virtual void onMouseChange()	{}; /// triggered once when a button is pressed or released; check here for <click> / <double-click> !
		virtual void onMouseScroll()	{};
		virtual void onMouseMove()		{};

		/// application focus change
		virtual void onFocusChange(bool focus) {}; /// => <true> = focused

		/// <TODO> : joystick events

		void acceptKeyEvents(bool);
		void acceptMouseEvents(bool);
		void acceptFocusEvents(bool);
		void acceptJoyEvents(bool);

		void querySceneOnClick(bool);
		void querySceneOnMove(bool); /// cpu-intensive

		friend class HIDBoss;
	};
}

#endif