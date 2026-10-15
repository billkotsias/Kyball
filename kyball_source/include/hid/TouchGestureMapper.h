// Maps touch pointers onto the mouse events HIDBoss already exposes.
// One implementation is shared by the Linux --touch flag and Android.

#pragma once
#ifndef KYBALL_TOUCH_GESTURE_MAPPER_H
#define KYBALL_TOUCH_GESTURE_MAPPER_H

#include <deque>

namespace P3D {

	class TouchGestureMapper {
	public:
		// Movement, in pixels, before a one-finger contact becomes an aim-drag
		// (relative mouse motion, no button) instead of a tap (left click).
		static const int DRAG_THRESHOLD_PX = 16;

		// Pinch distance change, in pixels, that produces one mouse-wheel notch.
		static const float PINCH_PIXELS_PER_WHEEL_NOTCH;

		// Wheel units passed to HIDBoss for one notch (Windows WHEEL_DELTA).
		static const int WHEEL_NOTCH = 120;

		struct Output {
			int relX;
			int relY;
			int scroll;
			unsigned int buttons;
		};

		static TouchGestureMapper& instance();

		void pointerDown(int id, float x, float y);
		void pointerMove(int id, float x, float y);
		void pointerUp(int id, float x, float y);
		void cancel();

		// One synthetic mouse frame. Always writes a sample.
		void consume(Output& out);

	private:
		struct Finger {
			int id;
			float x, y;
			float startX, startY;
			float lastX, lastY;
			bool down;
		};

		TouchGestureMapper();

		Finger* find(int id);
		void drop(int id);
		bool pastThreshold() const;

		Finger fingers[8];
		int fingerCount;
		int maxFingers;
		bool dragged;
		float lastPinchDistance;
		bool havePinch;
		std::deque<Output> queue;
	};

}

#endif
