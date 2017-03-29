// Fantasia Scripting Language - Runtime

#pragma once
#ifndef FANLIB_FSLRuntime_H
#define FANLIB_FSLRuntime_H

#include <deque>

namespace FANLib {

	class FSLObject;

	class FSLRuntime {

	private://private

		// Garbage Collection logic

		/// common wastebin for all FSLRuntime instances
		static std::deque<FSLObject*> bin;

		/// object's pointers reached zero
		static void addToGC(FSLObject*);

	public:

		// constructor
		//FSLRuntime();
		//virtual ~FSLRuntime();

		/// do GC now
		static void performGC();

		friend class FSLObject;
		friend class FSLString;
	};
}

#endif