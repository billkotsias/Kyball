////////////
// ShrinkIn2Event
////////////

#pragma once
#ifndef ShrinkIn2Event_H
#define ShrinkIn2Event_H

#include "Event.h"

namespace P3D {

	class ShrinkIn2Event : public Event {

	public:

		ShrinkIn2Event() : Event(Event::SHRINK_IN_2) {};

	};
}

#endif