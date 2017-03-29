////////////
// LostEvent
////////////

#pragma once
#ifndef LostEvent_H
#define LostEvent_H

#include "Event.h"

namespace P3D {

	class LostEvent : public Event {

	public:

		LostEvent() : Event(Event::LOST) {};

	};
}

#endif