#include "StdAfx.h"
#include "game\event\EventListener.h"

#include "game\event\EventManager.h"

namespace P3D {

	EventListener::~EventListener() {
		for (std::map<EventPair, bool>::iterator it = registeredEvents.begin(); it != registeredEvents.end(); ++it) {
			const EventPair& pair = it->first;
			pair.eMan()->removeListener(pair.event(), this);
		}
	}

	void EventListener::watchEvent(P3D::EventManager *eMan, P3D::Event::Type eventType) {
		eMan->addListener(eventType, this);
		registeredEvents[EventPair(eMan,eventType)] = true; /// register to remove on exit
	}

	void EventListener::forgetEvent(P3D::EventManager *eMan, P3D::Event::Type eventType) {
		eMan->removeListener(eventType, this);
		registeredEvents.erase(EventPair(eMan,eventType)); /// unregister
	}
}