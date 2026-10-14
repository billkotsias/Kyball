#include "hid/TouchGestureMapper.h"

#include <cmath>
#include <cstddef>

namespace P3D {

	const float TouchGestureMapper::PINCH_PIXELS_PER_WHEEL_NOTCH = 40.f;

	TouchGestureMapper& TouchGestureMapper::instance() {
		static TouchGestureMapper mapper;
		return mapper;
	}

	TouchGestureMapper::TouchGestureMapper()
		: fingerCount(0), maxFingers(0), dragged(false), lastPinchDistance(0.f), havePinch(false) {
		for (int i = 0; i < 8; ++i) fingers[i].down = false;
	}

	TouchGestureMapper::Finger* TouchGestureMapper::find(int id) {
		for (int i = 0; i < 8; ++i) {
			if (fingers[i].down && fingers[i].id == id) return &fingers[i];
		}
		return 0;
	}

	void TouchGestureMapper::drop(int id) {
		for (int i = 0; i < 8; ++i) {
			if (fingers[i].down && fingers[i].id == id) {
				fingers[i].down = false;
				if (fingerCount > 0) --fingerCount;
				return;
			}
		}
	}

	bool TouchGestureMapper::pastThreshold() const {
		for (int i = 0; i < 8; ++i) {
			if (!fingers[i].down) continue;
			float dx = fingers[i].x - fingers[i].startX;
			float dy = fingers[i].y - fingers[i].startY;
			if (dx * dx + dy * dy > (float)DRAG_THRESHOLD_PX * (float)DRAG_THRESHOLD_PX)
				return true;
		}
		return false;
	}

	void TouchGestureMapper::pointerDown(int id, float x, float y) {
		if (find(id)) {
			pointerMove(id, x, y);
			return;
		}
		for (int i = 0; i < 8; ++i) {
			if (fingers[i].down) continue;
			fingers[i].id = id;
			fingers[i].x = fingers[i].startX = fingers[i].lastX = x;
			fingers[i].y = fingers[i].startY = fingers[i].lastY = y;
			fingers[i].down = true;
			++fingerCount;
			if (fingerCount > maxFingers) maxFingers = fingerCount;
			if (fingerCount == 1) {
				dragged = false;
				havePinch = false;
				maxFingers = 1;
			}
			havePinch = false;
			return;
		}
	}

	void TouchGestureMapper::pointerMove(int id, float x, float y) {
		Finger* finger = find(id);
		if (!finger) return;
		finger->x = x;
		finger->y = y;
		if (pastThreshold()) dragged = true;
	}

	void TouchGestureMapper::pointerUp(int id, float x, float y) {
		Finger* finger = find(id);
		if (!finger) return;
		finger->x = x;
		finger->y = y;
		if (pastThreshold()) dragged = true;

		int before = fingerCount;
		int gestureFingers = maxFingers;
		bool wasDrag = dragged;
		drop(id);

		if (fingerCount == 0) {
			if (!wasDrag) {
				Output press;
				press.relX = 0;
				press.relY = 0;
				press.scroll = 0;
				press.buttons = (gestureFingers >= 2 || before >= 2) ? 2u : 1u; // RIGHT : LEFT
				Output release = press;
				release.buttons = 0;
				queue.push_back(press);
				queue.push_back(release);
			}
			dragged = false;
			maxFingers = 0;
			havePinch = false;
		}
	}

	void TouchGestureMapper::cancel() {
		for (int i = 0; i < 8; ++i) fingers[i].down = false;
		fingerCount = 0;
		maxFingers = 0;
		dragged = false;
		havePinch = false;
		queue.clear();
	}

	void TouchGestureMapper::consume(Output& out) {
		if (!queue.empty()) {
			out = queue.front();
			queue.pop_front();
			return;
		}

		out.relX = 0;
		out.relY = 0;
		out.scroll = 0;
		out.buttons = 0;

		if (fingerCount <= 0) return;

		if (pastThreshold()) dragged = true;

		float cx = 0.f, cy = 0.f, lx = 0.f, ly = 0.f;
		int n = 0;
		for (int i = 0; i < 8; ++i) {
			if (!fingers[i].down) continue;
			cx += fingers[i].x;
			cy += fingers[i].y;
			lx += fingers[i].lastX;
			ly += fingers[i].lastY;
			++n;
		}
		if (n <= 0) return;
		cx /= (float)n;
		cy /= (float)n;
		lx /= (float)n;
		ly /= (float)n;

		if (n >= 2) {
			float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
			int seen = 0;
			for (int i = 0; i < 8; ++i) {
				if (!fingers[i].down) continue;
				if (seen == 0) { x0 = fingers[i].x; y0 = fingers[i].y; }
				else if (seen == 1) { x1 = fingers[i].x; y1 = fingers[i].y; }
				++seen;
			}
			float dist = std::sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
			if (havePinch) {
				float delta = dist - lastPinchDistance;
				if (std::fabs(delta) >= 1.f) {
					// Pinch out (fingers apart) is a positive wheel notch: camera zooms in.
					out.scroll = (int)(delta / PINCH_PIXELS_PER_WHEEL_NOTCH * (float)WHEEL_NOTCH);
					if (std::fabs(delta) > (float)DRAG_THRESHOLD_PX) dragged = true;
				}
			}
			lastPinchDistance = dist;
			havePinch = true;
		} else {
			havePinch = false;
		}

		if (dragged) {
			out.relX = (int)(cx - lx);
			out.relY = (int)(cy - ly);
			if (n >= 2) out.buttons = 2u; // MOUSE_RIGHT held while orbiting
		}

		for (int i = 0; i < 8; ++i) {
			if (!fingers[i].down) continue;
			fingers[i].lastX = fingers[i].x;
			fingers[i].lastY = fingers[i].y;
		}
	}

}
