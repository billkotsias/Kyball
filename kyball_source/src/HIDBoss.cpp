#include "StdAfx.h"
#include "hid/HIDBoss.h"

#include "hid/HIDListener.h"
#include "hid/TouchGestureMapper.h"
#include "P3DWindow.h"
#include "OIS.h"

#include <SDL.h>

#include <cstring>

namespace P3D {

	static bool g_touchInput = false;
	static void* g_sdlWindow = 0;
	static int g_ignoreMotion = 0;

	static int sdlScancodeToOis(SDL_Scancode sc) {
		switch (sc) {
			case SDL_SCANCODE_ESCAPE: return OIS::KC_ESCAPE;
			case SDL_SCANCODE_1: return OIS::KC_1;
			case SDL_SCANCODE_2: return OIS::KC_2;
			case SDL_SCANCODE_3: return OIS::KC_3;
			case SDL_SCANCODE_4: return OIS::KC_4;
			case SDL_SCANCODE_5: return OIS::KC_5;
			case SDL_SCANCODE_6: return OIS::KC_6;
			case SDL_SCANCODE_7: return OIS::KC_7;
			case SDL_SCANCODE_8: return OIS::KC_8;
			case SDL_SCANCODE_9: return OIS::KC_9;
			case SDL_SCANCODE_0: return OIS::KC_0;
			case SDL_SCANCODE_MINUS: return OIS::KC_MINUS;
			case SDL_SCANCODE_EQUALS: return OIS::KC_EQUALS;
			case SDL_SCANCODE_BACKSPACE: return OIS::KC_BACK;
			case SDL_SCANCODE_TAB: return OIS::KC_TAB;
			case SDL_SCANCODE_Q: return OIS::KC_Q;
			case SDL_SCANCODE_W: return OIS::KC_W;
			case SDL_SCANCODE_E: return OIS::KC_E;
			case SDL_SCANCODE_R: return OIS::KC_R;
			case SDL_SCANCODE_T: return OIS::KC_T;
			case SDL_SCANCODE_Y: return OIS::KC_Y;
			case SDL_SCANCODE_U: return OIS::KC_U;
			case SDL_SCANCODE_I: return OIS::KC_I;
			case SDL_SCANCODE_O: return OIS::KC_O;
			case SDL_SCANCODE_P: return OIS::KC_P;
			case SDL_SCANCODE_LEFTBRACKET: return OIS::KC_LBRACKET;
			case SDL_SCANCODE_RIGHTBRACKET: return OIS::KC_RBRACKET;
			case SDL_SCANCODE_RETURN: return OIS::KC_RETURN;
			case SDL_SCANCODE_LCTRL: return OIS::KC_LCONTROL;
			case SDL_SCANCODE_A: return OIS::KC_A;
			case SDL_SCANCODE_S: return OIS::KC_S;
			case SDL_SCANCODE_D: return OIS::KC_D;
			case SDL_SCANCODE_F: return OIS::KC_F;
			case SDL_SCANCODE_G: return OIS::KC_G;
			case SDL_SCANCODE_H: return OIS::KC_H;
			case SDL_SCANCODE_J: return OIS::KC_J;
			case SDL_SCANCODE_K: return OIS::KC_K;
			case SDL_SCANCODE_L: return OIS::KC_L;
			case SDL_SCANCODE_SEMICOLON: return OIS::KC_SEMICOLON;
			case SDL_SCANCODE_APOSTROPHE: return OIS::KC_APOSTROPHE;
			case SDL_SCANCODE_GRAVE: return OIS::KC_GRAVE;
			case SDL_SCANCODE_LSHIFT: return OIS::KC_LSHIFT;
			case SDL_SCANCODE_BACKSLASH: return OIS::KC_BACKSLASH;
			case SDL_SCANCODE_Z: return OIS::KC_Z;
			case SDL_SCANCODE_X: return OIS::KC_X;
			case SDL_SCANCODE_C: return OIS::KC_C;
			case SDL_SCANCODE_V: return OIS::KC_V;
			case SDL_SCANCODE_B: return OIS::KC_B;
			case SDL_SCANCODE_N: return OIS::KC_N;
			case SDL_SCANCODE_M: return OIS::KC_M;
			case SDL_SCANCODE_COMMA: return OIS::KC_COMMA;
			case SDL_SCANCODE_PERIOD: return OIS::KC_PERIOD;
			case SDL_SCANCODE_SLASH: return OIS::KC_SLASH;
			case SDL_SCANCODE_RSHIFT: return OIS::KC_RSHIFT;
			case SDL_SCANCODE_SPACE: return OIS::KC_SPACE;
			case SDL_SCANCODE_CAPSLOCK: return OIS::KC_CAPITAL;
			case SDL_SCANCODE_F1: return OIS::KC_F1;
			case SDL_SCANCODE_F2: return OIS::KC_F2;
			case SDL_SCANCODE_F3: return OIS::KC_F3;
			case SDL_SCANCODE_F4: return OIS::KC_F4;
			case SDL_SCANCODE_F5: return OIS::KC_F5;
			case SDL_SCANCODE_F6: return OIS::KC_F6;
			case SDL_SCANCODE_F7: return OIS::KC_F7;
			case SDL_SCANCODE_F8: return OIS::KC_F8;
			case SDL_SCANCODE_F9: return OIS::KC_F9;
			case SDL_SCANCODE_F10: return OIS::KC_F10;
			case SDL_SCANCODE_F11: return OIS::KC_F11;
			case SDL_SCANCODE_F12: return OIS::KC_F12;
			case SDL_SCANCODE_UP: return OIS::KC_UP;
			case SDL_SCANCODE_DOWN: return OIS::KC_DOWN;
			case SDL_SCANCODE_LEFT: return OIS::KC_LEFT;
			case SDL_SCANCODE_RIGHT: return OIS::KC_RIGHT;
			case SDL_SCANCODE_HOME: return OIS::KC_HOME;
			case SDL_SCANCODE_END: return OIS::KC_END;
			case SDL_SCANCODE_PAGEUP: return OIS::KC_PGUP;
			case SDL_SCANCODE_PAGEDOWN: return OIS::KC_PGDOWN;
			case SDL_SCANCODE_INSERT: return OIS::KC_INSERT;
			case SDL_SCANCODE_DELETE: return OIS::KC_DELETE;
			case SDL_SCANCODE_LALT: return OIS::KC_LMENU;
			case SDL_SCANCODE_RALT: return OIS::KC_RMENU;
			case SDL_SCANCODE_RCTRL: return OIS::KC_RCONTROL;
			default: return -1;
		}
	}

	static void noteClosedWindow() {
		Ogre::RenderWindow* window = P3DWindow::getWindow();
		if (window && !window->isClosed()) window->destroy();
	}

	static void feedFingerEvent(const SDL_Event& event) {
		SDL_Window* window = (SDL_Window*)g_sdlWindow;
		int width = 1, height = 1;
		if (window) SDL_GetWindowSize(window, &width, &height);
		if (width < 1) width = 1;
		if (height < 1) height = 1;
		int id = (int)(event.tfinger.fingerId % 100000) + 10;
		float x = event.tfinger.x * (float)width;
		float y = event.tfinger.y * (float)height;
		TouchGestureMapper& mapper = TouchGestureMapper::instance();
		if (event.type == SDL_FINGERDOWN) mapper.pointerDown(id, x, y);
		else if (event.type == SDL_FINGERMOTION) mapper.pointerMove(id, x, y);
		else if (event.type == SDL_FINGERUP) mapper.pointerUp(id, x, y);
	}

	// Linux --touch: the mouse is finger 0. Shift adds a second finger with a
	// fixed offset (two-finger drag / double-tap). Alt mirrors that finger
	// around the press point so moving the mouse pinches.
	static void feedMouseAsTouch() {
		SDL_Window* window = (SDL_Window*)g_sdlWindow;
		if (!window) return;
		int x = 0, y = 0;
		Uint32 buttons = SDL_GetMouseState(&x, &y);
		bool down = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
		bool alt = keys[SDL_SCANCODE_LALT] || keys[SDL_SCANCODE_RALT];
		bool second = down && (shift || alt);

		static bool wasDown = false;
		static bool wasSecond = false;
		static float centerX = 0.f;
		static float centerY = 0.f;

		TouchGestureMapper& mapper = TouchGestureMapper::instance();
		if (down && !wasDown) mapper.pointerDown(0, (float)x, (float)y);
		else if (down) mapper.pointerMove(0, (float)x, (float)y);
		else if (wasDown) mapper.pointerUp(0, (float)x, (float)y);

		float sx, sy;
		if (alt) {
			if (!wasSecond) {
				centerX = (float)x;
				centerY = (float)y;
			}
			sx = centerX * 2.f - (float)x;
			sy = centerY * 2.f - (float)y;
		} else {
			sx = (float)x + 120.f;
			sy = (float)y;
		}
		if (second && !wasSecond) mapper.pointerDown(1, sx, sy);
		else if (second) mapper.pointerMove(1, sx, sy);
		else if (wasSecond) mapper.pointerUp(1, sx, sy);

		wasDown = down;
		wasSecond = second;
	}

	//
	// MouseHistory
	//

	MouseHistory::MouseHistory() {
		for (int i = 0; i < levels; ++i) {
			buttons[i] = 0;
			cycles[i] = 0;
			relX[i] = 0;
			relY[i] = 0;
		}
	}

	void MouseHistory::insertChange(unsigned int butts, unsigned int cyc, int x, int y) {
		for (int i = 0; i < levels - 1; ++i) {
			buttons[i] = buttons[i+1];
			cycles[i] = cycles[i+1];
			relX[i] = relX[i+1];
			relY[i] = relY[i+1];
		}
		buttons[levels - 1] = butts;
		cycles[levels - 1] = cyc;
		relX[levels - 1] = x;
		relY[levels - 1] = y;
	}

	bool MouseHistory::checkClick(unsigned int button, unsigned int dist) {
		if ( (buttons[levels - 2] & button) - (buttons[levels - 1] & button) == button ) {
			if ((unsigned int)abs(relX[levels - 1]) <= dist && (unsigned int)abs(relY[levels - 1]) <= dist)
				return true;
		}
		return false;
	}

	bool MouseHistory::checkDouble(unsigned int button, unsigned int dist, unsigned int dur) {
		if ( ( (buttons[levels - 2] & button) - (buttons[levels - 1] & button) == button ) &&
			 ( (buttons[levels - 4] & button) - (buttons[levels - 3] & button) == button ) ) {
			if (cycles[levels - 1] - cycles[levels - 4] <= dur) {
				unsigned int totalX = abs( relX[levels - 1] + relX[levels - 2] + relX[levels - 3] );
				unsigned int totalY = abs( relY[levels - 1] + relY[levels - 2] + relY[levels - 3] );
				if (totalX <= dist && totalY <= dist)
					return true;
			}
		}
		return false;
	}

	//
	// HIDBoss
	//

	HIDBoss* HIDBoss::instance = 0;

	HIDBoss* HIDBoss::getSingletonPtr() {
		if (!instance) instance = new HIDBoss();
		return instance;
	}

	HIDBoss::~HIDBoss() {
		instance = 0;
		removeAllListeners();
	}

	void HIDBoss::destroy() {
		if (instance) delete instance;
		instance = 0;
	}

	int HIDBoss::windowHandle;

	void HIDBoss::setSdlWindow(void* window) { g_sdlWindow = window; }
	void HIDBoss::setTouchInput(bool enabled) { g_touchInput = enabled; }
	bool HIDBoss::useTouchInput() { return g_touchInput; }

	bool HIDBoss::applicationFocused() {
		SDL_Window* window = (SDL_Window*)g_sdlWindow;
		if (!window) return true;
		Uint32 flags = SDL_GetWindowFlags(window);
		if (flags & SDL_WINDOW_INPUT_FOCUS) return true;
#if defined(__ANDROID__)
		return (flags & SDL_WINDOW_SHOWN) != 0;
#else
		return false;
#endif
	}

	HIDBoss::HIDBoss() : currentKeyState(0), previousFocus(true), clippingRectEnabled(false), ogreMovable(0), cycles(0), mouseHidden(false), mouseGrabbed(false) {
		_windowHandle = windowHandle;
		mPosX = 0;
		mPosY = 0;
		mMinX = 0;
		mMinY = 0;
		mMaxX = 800;
		mMaxY = 600;
		mRelX = 0;
		mRelY = 0;
		mScroll = 0;
		SDL_Window* window = (SDL_Window*)g_sdlWindow;
		if (window) {
			int width = 0, height = 0;
			SDL_GetWindowSize(window, &width, &height);
			if (width > 0 && height > 0) {
				mMaxX = width;
				mMaxY = height;
				mPosX = width / 2;
				mPosY = height / 2;
			}
		}
		for (int i = 511; i >= 0; --i) keyStates[i] = 0;
		for (int i = 255; i >= 0; --i) keyChanges[i] = 0;
	}

	void HIDBoss::setMouseGrabbed(bool grab) {
		mouseGrabbed = grab;
		if (g_sdlWindow) SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
	}

	void HIDBoss::setMouseHidden(bool hide) {
		mouseHidden = hide;
		SDL_ShowCursor(hide ? SDL_DISABLE : SDL_ENABLE);
	}

	bool HIDBoss::getMouseGrabbed() { return mouseGrabbed; }
	bool HIDBoss::getMouseHidden() { return mouseHidden; }

	void HIDBoss::removeAllListeners() {
		keyEventListeners.clear();
		mouseEventListeners.clear();
		changeFocusListeners.clear();
		joyEventListeners.clear();
		clickQueryListeners.clear();
		moveQueryListeners.clear();
	}

	void HIDBoss::addKeyEventListener(HIDListener* listener) { keyEventListeners[listener] = true; }
	void HIDBoss::removeKeyEventListener(HIDListener* listener) {
		keyEventListeners.erase(listener);
		exe_keyEventListeners.erase(listener);
	}
	void HIDBoss::addMouseEventListener(HIDListener* listener) { mouseEventListeners[listener] = true; }
	void HIDBoss::removeMouseEventListener(HIDListener* listener) {
		mouseEventListeners.erase(listener);
		exe_mouseEventListeners.erase(listener);
	}
	void HIDBoss::addFocusEventListener(HIDListener* listener) { changeFocusListeners[listener] = true; }
	void HIDBoss::removeFocusEventListener(HIDListener* listener) { changeFocusListeners.erase(listener); }
	void HIDBoss::addJoyEventListener(HIDListener* listener) { joyEventListeners[listener] = true; }
	void HIDBoss::removeJoyEventListener(HIDListener* listener) { joyEventListeners.erase(listener); }
	void HIDBoss::addClickQueryListener(HIDListener* listener) { clickQueryListeners[listener] = true; }
	void HIDBoss::removeClickQueryListener(HIDListener* listener) { clickQueryListeners.erase(listener); }
	void HIDBoss::addMoveQueryListener(HIDListener* listener) { moveQueryListeners[listener] = true; }
	void HIDBoss::removeMoveQueryListener(HIDListener* listener) { moveQueryListeners.erase(listener); }

	void HIDBoss::setClippingRect(int xmin, int ymin, int xmax, int ymax) {
		mMinX = xmin;
		mMinY = ymin;
		mMaxX = xmax;
		mMaxY = ymax;
	}

	template <void(*function)(HIDListener* listener)>
	void HIDBoss::iterateListeners(std::map<HIDListener*, bool>& listeners, std::map<HIDListener*, bool>& exe_listeners) {
		exe_listeners = listeners;
		while (exe_listeners.size()) {
			HIDListener* exeListener = exe_listeners.begin()->first;
			function(exeListener);
			if (exe_listeners.size() && exeListener == exe_listeners.begin()->first) exe_listeners.erase(exe_listeners.begin());
		}
	}
	void HIDBoss::onMouseChange(HIDListener* listener) { listener->onMouseChange(); };
	void HIDBoss::onMouseDown(HIDListener* listener) { listener->onMouseDown(); };
	void HIDBoss::onMouseScroll(HIDListener* listener) { listener->onMouseScroll(); };
	void HIDBoss::onMouseMove(HIDListener* listener) { listener->onMouseMove(); };
	void HIDBoss::onKeyEvent(HIDListener* listener) { listener->onKeyEvent(); };

	void HIDBoss::captureDevices(unsigned int runCycles) {
		cycles += runCycles;
		bool focus = applicationFocused();

		if (previousFocus != focus) {
			for (std::map<HIDListener*, bool>::iterator it = changeFocusListeners.begin(); it != changeFocusListeners.end(); ++it) {
				it->first->onFocusChange(focus);
			}
		}

		int relX = 0;
		int relY = 0;
		int scroll = 0;
		unsigned int mButtons = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) noteClosedWindow();
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) noteClosedWindow();
			if (g_touchInput && (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION || event.type == SDL_FINGERUP))
				feedFingerEvent(event);
			if (!g_touchInput && event.type == SDL_MOUSEMOTION) {
				if (g_ignoreMotion > 0) { --g_ignoreMotion; }
				else { relX += event.motion.xrel; relY += event.motion.yrel; }
			}
			if (!g_touchInput && event.type == SDL_MOUSEWHEEL) scroll += event.wheel.y * TouchGestureMapper::WHEEL_NOTCH;
		}

		if (!focus) {
			previousFocus = false;
			return;
		}
		if (!previousFocus) {
			previousFocus = true;
			return;
		}

		if (g_touchInput) {
#ifndef __ANDROID__
			feedMouseAsTouch();
#endif
			TouchGestureMapper::Output sample;
			TouchGestureMapper::instance().consume(sample);
			relX = sample.relX;
			relY = sample.relY;
			scroll = sample.scroll;
			mButtons = sample.buttons;
		} else if (g_sdlWindow) {
			Uint32 buttons = SDL_GetMouseState(NULL, NULL);
			if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) mButtons |= HIDBoss::MOUSE_LEFT;
			if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) mButtons |= HIDBoss::MOUSE_RIGHT;
			if (buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) mButtons |= HIDBoss::MOUSE_MIDDLE;
			if (buttons & SDL_BUTTON(SDL_BUTTON_X1)) mButtons |= HIDBoss::MOUSE_4;
			if (buttons & SDL_BUTTON(SDL_BUTTON_X2)) mButtons |= HIDBoss::MOUSE_5;
		}

		currentKeyState = 256 - currentKeyState;
		std::memset(keyStates + currentKeyState, 0, 256);
		const Uint8* keyState = SDL_GetKeyboardState(NULL);
		if (keyState) {
			for (int sc = SDL_SCANCODE_UNKNOWN; sc < SDL_NUM_SCANCODES; ++sc) {
				if (!keyState[sc]) continue;
				int code = sdlScancodeToOis((SDL_Scancode)sc);
				if (code >= 0 && code < 256) keyStates[currentKeyState + code] = 1;
			}
		}
		for (int i = 255; i >= 0; --i) {
			keyChanges[i] = keyStates[i + currentKeyState] - keyStates[i + 256 - currentKeyState];
		}
		iterateListeners<&onKeyEvent>(keyEventListeners, exe_keyEventListeners);

		mScroll = scroll;
		int newPosX = mPosX + relX;
		int newPosY = mPosY + relY;
		if (clippingRectEnabled) {
			if (newPosX < mMinX) newPosX = mMinX;
			else if (newPosX > mMaxX) newPosX = mMaxX;
			if (newPosY < mMinY) newPosY = mMinY;
			else if (newPosY > mMaxY) newPosY = mMaxY;
		}
		mRelX = newPosX - mPosX;
		mRelY = newPosY - mPosY;
		mPosX = newPosX;
		mPosY = newPosY;

		bool sceneQueried = false;

		if (mButtons != mHistory.lastButtons()) {
			mHistory.insertChange(mButtons, cycles, mRelX, mRelY);
			if (!sceneQueried && clickQueryListeners.size()) { sceneQueried = true; queryScene(); }
			iterateListeners<&onMouseChange>(mouseEventListeners, exe_mouseEventListeners);
		}

		if (mButtons) {
			if (clickQueryListeners.size()) { sceneQueried = true; queryScene(); }
			iterateListeners<&onMouseDown>(mouseEventListeners, exe_mouseEventListeners);
		}

		if (mScroll != 0) {
			if (!sceneQueried && clickQueryListeners.size()) { sceneQueried = true; queryScene(); }
			iterateListeners<&onMouseScroll>(mouseEventListeners, exe_mouseEventListeners);
		}

		if (mRelX != 0 || mRelY != 0) {
			if (!sceneQueried && moveQueryListeners.size()) { sceneQueried = true; queryScene(); }
			iterateListeners<&onMouseMove>(mouseEventListeners, exe_mouseEventListeners);
		}
	}

	void HIDBoss::queryScene() {
	}

	Ogre::MovableObject* HIDBoss::getQueryResult() { return ogreMovable; }

	bool HIDBoss::isKeyDown() {
		for (int i = 255; i >= 0; --i) {
			if (keyStates[i + currentKeyState]) return true;
		}
		return false;
	}

	bool HIDBoss::isKeyPressed() {
		for (int i = 255; i >= 0; --i) {
			if (keyChanges[i] == 1) return true;
		}
		return false;
	}

	bool HIDBoss::isKeyReleased() {
		for (int i = 255; i >= 0; --i) {
			if (keyChanges[i] == -1) return true;
		}
		return false;
	}

	bool HIDBoss::isButtonDown(P3D::HIDBoss::MouseButton id) {
		return ((mHistory.lastButtons() & id) != 0);
	}

	bool HIDBoss::isButtonPressed(P3D::HIDBoss::MouseButton id) {
		int buttonChanges = mHistory.lastChanges();
		return ( (buttonChanges & id) && (mHistory.lastButtons() & id) );
	}

	bool HIDBoss::isButtonPressed() {
		int buttonChanges = mHistory.lastChanges();
		return ( buttonChanges && (buttonChanges & mHistory.lastButtons()) );
	}

	bool HIDBoss::isButtonReleased(P3D::HIDBoss::MouseButton id) {
		int buttonChanges = mHistory.lastChanges();
		return ( (buttonChanges & id) && (mHistory.penultimateButtons() & id) );
	}

	bool HIDBoss::isButtonReleased() {
		int buttonChanges = mHistory.lastChanges();
		return ( buttonChanges && (buttonChanges & mHistory.penultimateButtons()) );
	}

	bool HIDBoss::isButtonClicked(HIDBoss::MouseButton id, unsigned int dist) {
		return mHistory.checkClick(id, dist);
	}

	bool HIDBoss::isButtonDClicked(HIDBoss::MouseButton id, unsigned int dist, unsigned int dur) {
		return mHistory.checkDouble(id, dist, dur);
	}

	int HIDBoss::getMouseScroll() { return mScroll; }

	void HIDBoss::getMouseMove(int &x, int &y) {
		x = mRelX;
		y = mRelY;
	}

	void HIDBoss::getMousePosition(int &x, int &y) {
		x = mPosX;
		y = mPosY;
	}

	void HIDBoss::setMousePosition(const int &x, const int &y) {
		mPosX = x;
		mPosY = y;
		if (!g_touchInput && g_sdlWindow) {
			g_ignoreMotion = 1;
			SDL_WarpMouseInWindow((SDL_Window*)g_sdlWindow, x, y);
		}
	}
}
