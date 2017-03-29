////////////////
// EventListener
////////////////

#pragma once
#ifndef EventListener_H
#define EventListener_H

#include "game\event\Event.h"
#include <map>

namespace P3D {

	class EventManager;

	//

	class EventPair {

	private:

		EventManager* _eMan;
		Event::Type _event;

	public:

		EventPair() : _eMan(0), _event(Event::_none) {};
		EventPair(EventManager *__eMan, Event::Type _evType) : _eMan(__eMan), _event(_evType) {};

		inline EventManager* eMan() const { return _eMan; };
		inline Event::Type event() const { return _event; };

		inline bool operator<(const EventPair &other) const {
			if ((void*)_eMan != other.eMan()) return ((void*)_eMan < other.eMan());
			return (_event < other.event());
		};
		inline bool operator<=(const EventPair &other) const {
			if ((void*)_eMan != other.eMan()) return ((void*)_eMan < other.eMan());
			return (_event <= other.event());
		};
		inline bool operator==(const EventPair &other) const {
			return ((void*)_eMan == other.eMan() && _event == other.event());
		};
	};

	//

	class EventListener {

	private:

		std::map<EventPair, bool> registeredEvents;

	protected:

		/// clean-code functions
		void watchEvent(EventManager*, Event::Type);
		void forgetEvent(EventManager*, Event::Type);

	public:

		virtual void incomingEvent(Event*) = 0; /// maybe it will be needed in the future to pass the 'EventManager' of this event..? (probably not)

		~EventListener();

	};
}

#endif