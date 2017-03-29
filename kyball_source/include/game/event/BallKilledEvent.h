//////////////////
// BallKilledEvent - a ball declares itself as DEAD
//////////////////

#pragma once
#ifndef BallKilledEvent_H
#define BallKilledEvent_H

#include "GOEvent.h"

namespace P3D {

	class GameplayObject;

	class BallKilledEvent : public GOEvent {

	public:

		BallKilledEvent(GameplayObject* inObj) : GOEvent(Event::BALL_KILLED, inObj) {};

		/// overrides
		virtual Event* copy() { return new BallKilledEvent(obj); };

	};
}

#endif