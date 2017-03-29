//////////////
// UnlinkEvent
//////////////

#pragma once
#ifndef UnlinkEvent_H
#define UnlinkEvent_H

#include "GOEvent.h"

namespace P3D {

	class UnlinkEvent : public GOEvent {

	public:

		UnlinkEvent(GameplayObject* inObj) : GOEvent(Event::UNLINK_ME, inObj) {};

		/// overrides
		virtual Event* copy() { return new UnlinkEvent(obj); };

	};
}

#endif