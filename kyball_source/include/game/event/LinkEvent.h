////////////
// LinkEvent
////////////

#pragma once
#ifndef LinkEvent_H
#define LinkEvent_H

#include "GOEvent.h"

namespace P3D {

	class LinkEvent : public GOEvent {

	public:

		LinkEvent(GameplayObject* inObj) : GOEvent(Event::LINK_ME, inObj) {};

		/// overrides
		virtual Event* copy() { return new LinkEvent(obj); };

	};
}

#endif