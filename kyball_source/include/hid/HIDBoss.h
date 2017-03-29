//////////
// HIDBoss - OIS/Ogre combiner
//////////

#pragma once
#ifndef HIDBoss_H
#define HIDBoss_H

#include <map>

namespace OIS {
	class InputManager;
	class Mouse;
	class Keyboard;
	class JoyStick;
}

namespace Ogre {
	class MovableObject;
}

namespace P3D {

	class HIDListener;

	/// Track Mouse changes history
	class MouseHistory {

	private:

		static const int levels = 4;	/// levels recorded

		unsigned int buttons[levels];	/// buttons pressed
		unsigned int cycles[levels];	/// cycles-count when change was registered
		int relX[levels];				/// } relative mouse translation between changes
		int relY[levels];				/// }

	public:

		/// constructor
		MouseHistory();

		void insertChange(unsigned int, unsigned int, int, int);

		/// check if a mouse button has been clicked/double-clicked
		/// =>	mouse code, as in HIDBoss::MouseButton
		///		maximum acceptable distance
		///		maximum acceptable duration (double-click only)
		bool checkClick(unsigned int, unsigned int);
		bool checkDouble(unsigned int, unsigned int, unsigned int);

		/// getters
		inline unsigned int lastButtons() { return buttons[levels - 1]; };
		inline unsigned int penultimateButtons() { return buttons[levels - 2]; };
		inline unsigned int lastChanges() { return buttons[levels - 1] ^ buttons[levels - 2]; };

	};

	class HIDBoss {

	private:

		HIDBoss();
		static HIDBoss* instance;
		~HIDBoss();

		///OIS Input devices
		OIS::InputManager*	inputManager;
		OIS::Mouse*			mouse;
		OIS::Keyboard*		keyboard;
		//TODO: OIS::JoyStick*		joystick;

		/// track keyboard
		char keyStates[512];	/// double-buffered
		int currentKeyState;	/// either 0 or 256
		signed char keyChanges[256];

		/// track focus
		bool previousFocus;

		/// track mouse
		MouseHistory mHistory;			/// button-based history
		int mScroll;					/// scroll button
		int mPosX, mPosY;				/// mouse position within clipping rectangle
		int mRelX, mRelY;				/// relative movement
		int mMinX, mMinY, mMaxX, mMaxY;	/// clipping rectangle inside app's window

		/// track scene queries
		Ogre::MovableObject* ogreMovable;

		/// track time
		unsigned int cycles; /// count of 'captureDevices()' calls

		/// Listeners
		std::map<HIDListener*, bool> keyEventListeners;
		std::map<HIDListener*, bool> mouseEventListeners;
		std::map<HIDListener*, bool> changeFocusListeners;
		std::map<HIDListener*, bool> joyEventListeners;
		std::map<HIDListener*, bool> clickQueryListeners;
		std::map<HIDListener*, bool> moveQueryListeners;

		/// - fix
		std::map<HIDListener*, bool> exe_mouseEventListeners;
		std::map<HIDListener*, bool> exe_keyEventListeners;
		template <void(*function)(HIDListener* listener)>
		void iterateListeners(std::map<HIDListener*, bool>& listeners, std::map<HIDListener*, bool>& exe_listeners);
		static void onMouseChange(HIDListener* listener);
		static void onMouseDown(HIDListener* listener);
		static void onMouseScroll(HIDListener* listener);
		static void onMouseMove(HIDListener* listener);
		static void onKeyEvent(HIDListener* listener);

		void queryScene();

		int _windowHandle;

	public:

		enum MouseButton {
			MOUSE_LEFT		= 1,
			MOUSE_RIGHT		= 2,
			MOUSE_MIDDLE	= 4,
			MOUSE_4			= 8,
			MOUSE_5			= 16,
			MOUSE_6			= 32,
			MOUSE_7			= 64,
			MOUSE_8			= 128,
		};

		static int windowHandle; /// set this before calling HIDBoss for the first time

		static HIDBoss* getSingletonPtr();
		static void destroy();

		bool clippingRectEnabled;
		void setClippingRect(int, int, int, int);

		void	addKeyEventListener(HIDListener*);
		void removeKeyEventListener(HIDListener*);
		void	addMouseEventListener(HIDListener*);
		void removeMouseEventListener(HIDListener*);
		void	addFocusEventListener(HIDListener*);
		void removeFocusEventListener(HIDListener*);
		void	addJoyEventListener(HIDListener*);
		void removeJoyEventListener(HIDListener*);

		void	addClickQueryListener(HIDListener*);
		void removeClickQueryListener(HIDListener*);
		void	addMoveQueryListener(HIDListener*);
		void removeMoveQueryListener(HIDListener*);

		void removeAllListeners();

		/// read HIDs and trigger respective events
		/// => "cycles" since last run (to measure time)
		void captureDevices(unsigned int = 1);
		void setMouseHidden(bool);
		bool getMouseHidden();
		void setMouseGrabbed(bool);
		bool getMouseGrabbed();

		/// queries for 'HIDListener's to use
		/// keyboard
		/// - is key held down? (event continuously triggered)
		inline bool isKeyDown(unsigned char keycode) { return (keyStates[keycode + currentKeyState] != 0); };	/// specific key
		bool isKeyDown();																						/// any key
		/// - is key pressed? (event triggered once)
		inline bool isKeyPressed(unsigned char keycode) { return (keyChanges[keycode] == 1); };					/// specific key
		bool isKeyPressed();																					/// any key
		/// - is key released? (event triggered once)
		inline bool isKeyReleased(unsigned char keycode) { return (keyChanges[keycode] == -1); };				///	 >> >>
		bool isKeyReleased();

		/// mouse
		/// - is button held down? (event continuously triggered)
		bool isButtonDown(HIDBoss::MouseButton);
		/// - is button pressed? (event triggered once)
		bool isButtonPressed(HIDBoss::MouseButton);
		bool isButtonPressed();
		/// - is button released? (event triggered once)
		bool isButtonReleased(HIDBoss::MouseButton);
		bool isButtonReleased();
		/// - is button clicked / double-clicked? (event triggered once)
		bool isButtonClicked(HIDBoss::MouseButton, unsigned int);					/// => button id, maximum distance
		bool isButtonDClicked(HIDBoss::MouseButton, unsigned int, unsigned int);	/// => button id, maximum distance, maximum duration
		/// - get mouse scroll
		int getMouseScroll();
		/// - get relative movement
		void getMouseMove(int &, int &);
		/// - get Ogre object
		Ogre::MovableObject* getQueryResult();
		/// - position
		void getMousePosition(int &, int &);
		void setMousePosition(const int &, const int &);

		/// get "time"
		inline unsigned int getCycles() { return cycles; };
	};
}

#endif