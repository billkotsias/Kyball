///////////////
// EventManager
///////////////

#pragma once
#ifndef EventManager_H
#define EventManager_H

#include <map>

#include "Event.h"

namespace P3D {

	class EventListener;

	class EventManager {

	private:

		std::map< Event::Type, std::map< EventListener*, bool > > eventMap;

	public:

		void addListener(Event::Type, EventListener*);		/// add listener for this type of event
		void removeListener(Event::Type, EventListener*);	/// remove listener for this type of event

		void clear();	/// reset

		void incomingEvent(Event*);							/// dispatch this event to anyone listening for this type of event

	};
}

#endif