/////////////////
// CosmosFinEvent
/////////////////

#pragma once
#ifndef CosmosFinEvent_H
#define CosmosFinEvent_H

#include "Event.h"

namespace P3D {

	class CosmosFinEvent : public Event {

	public:

		CosmosFinEvent() : Event(Event::COSMOS_FINISHED) {};

	};
}

#endif