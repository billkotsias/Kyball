////////
// CannonShotEvent
////////

#pragma once
#ifndef CannonShotEvent_H
#define CannonShotEvent_H

#include "Event.h"

namespace P3D {

	class CannonShotEvent : public Event {

	public:

		CannonShotEvent() : Event(CANNON_SHOT) {};

	};
}

#endif