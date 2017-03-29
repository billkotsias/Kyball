//////////////////
// InPlaceEvent - an object declares that it has just moved "in-place"
//////////////////

#pragma once
#ifndef InPlaceEvent_H
#define InPlaceEvent_H

#include "GOEvent.h"

namespace P3D {

	class GameplayObject;

	class InPlaceEvent : public GOEvent {

	public:

		InPlaceEvent(GameplayObject* inObj) : GOEvent(Event::IN_PLACE, inObj) {};

		/// overrides
		virtual Event* copy() { return new InPlaceEvent(obj); };

	};
}

#endif