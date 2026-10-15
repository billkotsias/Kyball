// Fantasia Scripting Language - Variables types

#pragma once
#ifndef FANLIB_FSLTypes_H
#define FANLIB_FSLTypes_H

#include <string.h>

namespace FANLib {

	namespace FSL {

		// One script slot holds an int, a real, or a pointer. A float is wide
		// enough for a pointer only in the original 32-bit build.
		typedef double real;

		template <class T>
		inline real packSlot(const T& value) {
			real slot = 0;
			size_t bytes = sizeof(T);
			if (bytes > sizeof(slot)) bytes = sizeof(slot);
			memcpy(&slot, &value, bytes);
			return slot;
		}

		/// VAR_TYPE can be contained within a byte
		enum VAR_TYPE {

			NULL_TYPE = 0,

			/// base types
			INT		= 0x01,
			REAL	= 0x02,
			STRING	= 0x03,
			CLASS	= 0x04,
			ARRAY	= 0x05,

			/// unique types
			UQ_INT		= 0x81,
			UQ_REAL		= 0x82,
			UQ_STRING	= 0x83,

			TYPE_BITS = 0x0F,	/// up to 14(?) different types

			/// boolean bits - if all contained within a byte, it's just 4 bits
			UNIQUE	= 0x80,

			/// redundant types
			///BYTE,
			///DOUBLE,
		};

	}
}

#endif