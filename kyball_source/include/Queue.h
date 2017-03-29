////////
// Queue - queue objects!
////////

#pragma once
#ifndef Queue_H
#define Queue_H

#include <core\CallBack.h>
#include <deque>

namespace P3D {

	class Queue {

	protected:

		std::deque<FANLib::ICallBack*> objects;

	public:

		~Queue();

		template <class Receiver>
		void pushBack(Receiver* receiver, void(Receiver::*function)(void*)) {
			objects.push_back( new FANLib::CallBack<Receiver>(receiver, function, (Queue*)(this)) );
		}

		template <class Receiver>
		void pushFront(Receiver* receiver, void(Receiver::*function)(void*)) {
			objects.push_front( new FANLib::CallBack<Receiver>(receiver, function, (Queue*)(this)) );
		}

		// STUPID LOGIC : It should be "*I* have finished", and *IF* this guy is at the front of the queue, we move to the next one, else FRIGGING IGNORE HIM
		void next();
		void reset();
	};
}

#endif