/////////////
// MoverEvent
/////////////

#pragma once
#ifndef MoverEvent_H
#define MoverEvent_H

#include "GOEvent.h"

namespace P3D {

	class MoverEvent : public GOEvent {

	public:

		MoverEvent(GameplayObject* inObj) : GOEvent(Event::MOVER_EVENT, inObj) {};

		/// overrides
		virtual Event* copy() { return new MoverEvent(obj); };

	};
}

#endif