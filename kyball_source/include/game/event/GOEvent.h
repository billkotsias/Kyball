//////////
// GOEvent - base Event class carrying a 'GameplayObject'
//////////

#pragma once
#ifndef GOEvent_H
#define GOEvent_H

#include "Event.h"

namespace P3D {

	class GameplayObject;

	class GOEvent : public Event {

	protected:

		GameplayObject* obj;

		GOEvent(Event::Type eventType, GameplayObject* inObj) : Event(eventType), obj(inObj) {};

	public:

		inline GameplayObject* getGameplayObject() { return obj; };

		/// overrides
		virtual Event* copy() = 0;

	};
}

#endif