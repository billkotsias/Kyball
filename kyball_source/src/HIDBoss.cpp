#include "StdAfx.h"
#include "hid\HIDBoss.h"

#include "hid\HIDListener.h"
#include <OIS.h>
#include <string>
#include <sstream>
#include <WinUser.h>

namespace P3D {

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
		/// shift history
		for (int i = 0; i < levels - 1; ++i) {
			buttons[i] = buttons[i+1];
			cycles[i] = cycles[i+1];
			relX[i] = relX[i+1];
			relY[i] = relY[i+1];
		}
		/// new registry
		buttons[levels - 1] = butts;
		cycles[levels - 1] = cyc;
		relX[levels - 1] = x;
		relY[levels - 1] = y;
	}

	bool MouseHistory::checkClick(unsigned int button, unsigned int dist) {

		/// was button pressed and released?
		if ( (buttons[levels - 2] & button) - (buttons[levels - 1] & button) == button ) {

			/// within space-frame?
			if ((unsigned int)abs(relX[levels - 1]) <= dist && (unsigned int)abs(relY[levels - 1]) <= dist)
				return true;
		}

		return false;
	}

	bool MouseHistory::checkDouble(unsigned int button, unsigned int dist, unsigned int dur) {

		/// was button pressed and released, twice?
		if ( ( (buttons[levels - 2] & button) - (buttons[levels - 1] & button) == button ) &&
			 ( (buttons[levels - 4] & button) - (buttons[levels - 3] & button) == button ) ) {

			/// within time-frame?
			if (cycles[levels - 1] - cycles[levels - 4] <= dur) {

				/// within space-frame?
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

	// Singleton logic
	HIDBoss* HIDBoss::instance = 0;

	HIDBoss* HIDBoss::getSingletonPtr() {
		if (!instance) instance = new HIDBoss();
		return instance;
	}

	HIDBoss::~HIDBoss() {

		instance = 0;

		removeAllListeners();
		if (keyboard)	inputManager->destroyInputObject(keyboard);
		if (mouse)		inputManager->destroyInputObject(mouse);
		/// TODO : if (joystick)	inputManager->destroyInputObject(joystick);

		OIS::InputManager::destroyInputSystem(inputManager);
	}

	void HIDBoss::destroy() {
		if (instance) delete instance;
		instance = 0;
	}
	// Singleton logic

	int HIDBoss::windowHandle; /// set this before calling HIDBoss for the first time

	HIDBoss::HIDBoss() : currentKeyState(0), previousFocus(true), clippingRectEnabled(false), ogreMovable(0), cycles(0) {

		_windowHandle = windowHandle;

		/// init OIS
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		windowHndStr << _windowHandle;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		inputManager = OIS::Win32InputManager::createInputSystem( pl );

		keyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));

		mouse = static_cast<OIS::Mouse*>(inputManager->createInputObject(OIS::OISMouse, true));
		mouse->capture();
		OIS::MouseState mouseState = mouse->getMouseState();
		mPosX = mouseState.X.abs;
		mPosY = mouseState.Y.abs;
		mMinX = 0;
		mMinY = 0;
		mMaxX = mouseState.width;
		mMaxY = mouseState.height;
		mRelX = 0;
		mRelY = 0;

		/// TODO : <joystick>

		/// zero states and changes
		for (int i = 511; i >= 0; --i) {
			keyStates[i] = 0;
		}
		for (int i = 255; i >= 0; --i) {
			keyChanges[i] = 0;
		}
	}

	void HIDBoss::setMouseGrabbed(bool grab) {
		mouse->grab(grab);
	}

	void HIDBoss::setMouseHidden(bool hide) {
		mouse->hide(hide);
	}

	bool HIDBoss::getMouseGrabbed() {
		return mouse->isGrabbed();
	}

	bool HIDBoss::getMouseHidden() {
		return mouse->isHidden();
	}

	void HIDBoss::removeAllListeners() {
		keyEventListeners.clear();
		mouseEventListeners.clear();
		changeFocusListeners.clear();
		joyEventListeners.clear();

		clickQueryListeners.clear();
		moveQueryListeners.clear();
	}

	void HIDBoss::addKeyEventListener(HIDListener* listener) {
		keyEventListeners[listener] = true;
	}
	void HIDBoss::removeKeyEventListener(HIDListener* listener) {
		keyEventListeners.erase(listener);
		exe_keyEventListeners.erase(listener); /// <fix>
	}
	void HIDBoss::addMouseEventListener(HIDListener* listener) {
		mouseEventListeners[listener] = true;
	}
	void HIDBoss::removeMouseEventListener(HIDListener* listener) {
		mouseEventListeners.erase(listener);
		exe_mouseEventListeners.erase(listener); /// <fix>
	}
	void HIDBoss::addFocusEventListener(HIDListener* listener) {
		changeFocusListeners[listener] = true;
	}
	void HIDBoss::removeFocusEventListener(HIDListener* listener) {
		changeFocusListeners.erase(listener);
	}
	void HIDBoss::addJoyEventListener(HIDListener* listener) {
		joyEventListeners[listener] = true;
	}
	void HIDBoss::removeJoyEventListener(HIDListener* listener) {
		joyEventListeners.erase(listener);
	}

	void HIDBoss::addClickQueryListener(HIDListener* listener) {
		clickQueryListeners[listener] = true;
	}
	void HIDBoss::removeClickQueryListener(HIDListener* listener) {
		clickQueryListeners.erase(listener);
	}
	void HIDBoss::addMoveQueryListener(HIDListener* listener) {
		moveQueryListeners[listener] = true;
	}
	void HIDBoss::removeMoveQueryListener(HIDListener* listener) {
		moveQueryListeners.erase(listener);
	}

	void HIDBoss::setClippingRect(int xmin, int ymin, int xmax, int ymax) {
		mMinX = xmin;
		mMinY = ymin;
		mMaxX = xmax;
		mMaxY = ymax;
	}

	template <void(*function)(HIDListener* listener)>
	void HIDBoss::iterateListeners(std::map<HIDListener*, bool>& listeners, std::map<HIDListener*, bool>& exe_listeners) {
		exe_listeners = listeners; /// copy to temp map
		while (exe_listeners.size()) {
			HIDListener* exeListener = exe_listeners.begin()->first;
			function(exeListener); /// during this function, the whole <exe_listeners> map may have been emptied! GET IT?!
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
		bool focus = ((int)GetForegroundWindow() == (int)_windowHandle);

		// application focus check
		if (previousFocus != focus) {
			for (std::map<HIDListener*, bool>::iterator it = changeFocusListeners.begin(); it != changeFocusListeners.end(); ++it) {
				it->first->onFocusChange(focus);
			}
		}

		if (!focus) {
			previousFocus = false;
			return; /// if application not focused, don't capture devices and don't trigger relevant events
		}

		if (!previousFocus) {

			/// OIS bug-fixes
			mouse->capture();												/// don't report 1st mouse buttons click after regaining focus	} KEEP THIS ORDER
			const_cast<OIS::MouseState &>(mouse->getMouseState()).clear();	/// don't falsely report mouse button held down					}		!!!

			previousFocus = true;
			return; /// we have just regained focus, so don't capture devices and don't trigger relevant events
		}

		// capture all available devices
		/// <TODO> : if (joystick)	joystick->capture();

		// keyboard
		if (keyboard)	keyboard->capture();
		/// track changes
		currentKeyState = 256 - currentKeyState;
		keyboard->copyKeyStates((char*)((int)keyStates + currentKeyState));
		for (int i = 255; i >= 0; --i) {
			keyChanges[i] = keyStates[i + currentKeyState] - keyStates[i + 256 - currentKeyState];
		}
		/// trigger events
		iterateListeners<&onKeyEvent>(keyEventListeners, exe_keyEventListeners);
		//for (std::map<HIDListener*, bool>::iterator it = keyEventListeners.begin(); it != keyEventListeners.end(); ++it) {
		//	it->first->onKeyEvent();
		//}

		// mouse
		if (mouse)		mouse->capture();
		/// track changes
		OIS::MouseState mouseState = mouse->getMouseState();
		//unsigned int mButtons = mouseState.buttons; /// was!
		unsigned int mButtons = 0; /// <override OIS bug with mouse buttons !!!
		if (GetAsyncKeyState(VK_LBUTTON) < 0) mButtons |= HIDBoss::MOUSE_LEFT;
		if (GetAsyncKeyState(VK_RBUTTON) < 0) mButtons |= HIDBoss::MOUSE_RIGHT;
		if (GetAsyncKeyState(VK_MBUTTON) < 0) mButtons |= HIDBoss::MOUSE_MIDDLE;
		if (GetAsyncKeyState(VK_XBUTTON1) < 0) mButtons |= HIDBoss::MOUSE_4;
		if (GetAsyncKeyState(VK_XBUTTON2) < 0) mButtons |= HIDBoss::MOUSE_5;
		mScroll = mouseState.Z.rel;
		int newPosX = mPosX + mouseState.X.rel;
		int newPosY = mPosY + mouseState.Y.rel;
		if (clippingRectEnabled) {
			if (newPosX < mMinX) {
				newPosX = mMinX;
			} else if (newPosX > mMaxX) {
				newPosX = mMaxX;
			}
			if (newPosY < mMinY) {
				newPosY = mMinY;
			} else if (newPosY > mMaxY) {
				newPosY = mMaxY;
			}
		}
		mRelX = newPosX - mPosX;
		mRelY = newPosY - mPosY;
		mPosX = newPosX;
		mPosY = newPosY;

		bool sceneQueried = false;

		/// 1st - change in buttons' states?
		if (mButtons != mHistory.lastButtons()) {
			mHistory.insertChange(mButtons, cycles, mRelX, mRelY);
			if (!sceneQueried && clickQueryListeners.size()) { sceneQueried = true; queryScene(); }
			iterateListeners<&onMouseChange>(mouseEventListeners, exe_mouseEventListeners);
		}

		/// 2nd - currently pressed buttons
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
		/// TODO : do a mouse-ray scene query and collect all returned 'MovableObject's, then get closest one (simple implementation)
		/// TODO : pixel-perfect scene query
	}

	Ogre::MovableObject* HIDBoss::getQueryResult() {
		return ogreMovable;
	}

	//bool HIDBoss::isKeyDown(unsigned char keycode) {
	//	return (keyStates[keycode + currentKeyState] != 0);
	//}
	bool HIDBoss::isKeyDown() {
		for (int i = 255; i >= 0; --i) {
			if (keyStates[i + currentKeyState]) return true;
		}
		return false;
	}

	//bool HIDBoss::isKeyPressed(unsigned char keycode) {
	//	return (keyChanges[keycode] == 1);
	//}
	bool HIDBoss::isKeyPressed() {
		for (int i = 255; i >= 0; --i) {
			if (keyChanges[i] == 1) return true;
		}
		return false;
	}

	//bool HIDBoss::isKeyReleased(unsigned char keycode) {
	//	return (keyChanges[keycode] == -1);
	//}
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
		//was: return ( (mButtons & id) != 0 && (mButtons_old & id) == 0 );
	}

	bool HIDBoss::isButtonPressed() {
		int buttonChanges = mHistory.lastChanges();
		return ( buttonChanges && (buttonChanges & mHistory.lastButtons()) );
		//was: return ( mButtons != 0 && (mButtons_old & mButtons) != mButtons );
	}

	bool HIDBoss::isButtonReleased(P3D::HIDBoss::MouseButton id) {
		int buttonChanges = mHistory.lastChanges();
		return ( (buttonChanges & id) && (mHistory.penultimateButtons() & id) );
		//was: return ( (mButtons & id) == 0 && (mButtons_old & id) != 0 );
	}

	bool HIDBoss::isButtonReleased() {
		int buttonChanges = mHistory.lastChanges();
		return ( buttonChanges && (buttonChanges & mHistory.penultimateButtons()) );
		//return ( mButtons_old != 0 && (mButtons & mButtons_old) != mButtons_old );
	}

	bool HIDBoss::isButtonClicked(HIDBoss::MouseButton id, unsigned int dist) {
		return mHistory.checkClick(id, dist);
	}

	bool HIDBoss::isButtonDClicked(HIDBoss::MouseButton id, unsigned int dist, unsigned int dur) {
		return mHistory.checkDouble(id, dist, dur);
	}

	int HIDBoss::getMouseScroll() {
		return mScroll;
	}

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
	}
}