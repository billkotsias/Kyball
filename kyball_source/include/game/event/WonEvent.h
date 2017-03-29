////////////
// WonEvent
////////////

#pragma once
#ifndef WonEvent_H
#define WonEvent_H

#include "Event.h"

namespace P3D {

	class WonEvent : public Event {

	public:

		WonEvent() : Event(Event::WON) {};

	};
}

#endif