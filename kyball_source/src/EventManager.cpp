#include "StdAfx.h"
#include "game\event\EventManager.h"
#include "game\event\EventListener.h"

namespace P3D {

	void EventManager::incomingEvent(Event* event) {

		/// transmit event to listeners listening for this type of event
		/// <NOTE> : anyone receiving this event who might need it for later use, should make a <copy> right-away because I'll <delete> it right-away!

		std::map< Event::Type, std::map< EventListener*, bool > >::iterator it = eventMap.find(event->getType());
		if (it != eventMap.end()) {
			for (std::map< EventListener*, bool >::iterator i = it->second.begin(); i != it->second.end(); ++i) {
				i->first->incomingEvent(event);
			}
		}

		delete event; /// <!!!!!!!!!!> for convenience to the programmer!
	}

	void EventManager::addListener(Event::Type type, EventListener* listener) {

		/// check if there are already listeners for this type of event, if not add a new <map> of listeners
		std::map< Event::Type, std::map< EventListener*, bool > >::iterator it = eventMap.find(type);
		if (it == eventMap.end()) {
			std::pair<Event::Type, std::map< EventListener*, bool> > pair(type, std::map< EventListener*, bool>());
			it = eventMap.insert(pair).first;
		}

		/// add listener to according map
		it->second[listener] = true;
	}

	void EventManager::removeListener(Event::Type type, EventListener* listener) {

		/// check if there are already listeners for this type of event, if not, someone is pulling our leg
		std::map< Event::Type, std::map< EventListener*, bool > >::iterator it = eventMap.find(type);
		if (it == eventMap.end()) return;

		/// remove listener from according map
		it->second.erase(listener);
	}

	void EventManager::clear() {
		eventMap.clear();
	}

}