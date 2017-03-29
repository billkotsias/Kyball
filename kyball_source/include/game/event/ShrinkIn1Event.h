////////////
// ShrinkIn1Event
////////////

#pragma once
#ifndef ShrinkIn1Event_H
#define ShrinkIn1Event_H

#include "Event.h"

namespace P3D {

	class ShrinkIn1Event : public Event {

	public:

		ShrinkIn1Event() : Event(Event::SHRINK_IN_1) {};

	};
}

#endif