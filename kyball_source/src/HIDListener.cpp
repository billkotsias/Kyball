#include "StdAfx.h"
#include "hid\HIDListener.h"

#include "hid\HIDBoss.h"

namespace P3D {

	HIDListener::HIDListener(bool key, bool mouse, bool focus, bool joy, bool clickQuery, bool moveQuery) {
		acceptKeyEvents(key);
		acceptMouseEvents(mouse);
		acceptFocusEvents(focus);
		acceptJoyEvents(joy);

		querySceneOnClick(clickQuery);
		querySceneOnMove(moveQuery);
	}

	HIDListener::~HIDListener() {
		acceptKeyEvents(false);
		acceptMouseEvents(false);
		acceptFocusEvents(false);
		acceptJoyEvents(false);

		querySceneOnClick(false);
		querySceneOnMove(false);
	}

	void HIDListener::acceptKeyEvents(bool key) {
		if (key) {
			HIDBoss::getSingletonPtr()->addKeyEventListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeKeyEventListener(this);
		}
	}

	void HIDListener::acceptMouseEvents(bool mouse) {
		if (mouse) {
			HIDBoss::getSingletonPtr()->addMouseEventListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeMouseEventListener(this);
		}
	}

	void HIDListener::acceptFocusEvents(bool focus) {
		if (focus) {
			HIDBoss::getSingletonPtr()->addFocusEventListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeFocusEventListener(this);
		}
	}

	void HIDListener::acceptJoyEvents(bool joy) {
		if (joy) {
			HIDBoss::getSingletonPtr()->addJoyEventListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeJoyEventListener(this);
		}
	}

	void HIDListener::querySceneOnClick(bool click) {
		if (click) {
			HIDBoss::getSingletonPtr()->addClickQueryListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeClickQueryListener(this);
		}
	}

	void HIDListener::querySceneOnMove(bool move) {
		if (move) {
			HIDBoss::getSingletonPtr()->addMoveQueryListener(this);
		} else {
			HIDBoss::getSingletonPtr()->removeMoveQueryListener(this);
		}
	}
}