// Fantasia Scripting Language - Variables types

#pragma once
#ifndef FANLIB_FSLTypes_H
#define FANLIB_FSLTypes_H

namespace FANLib {

	namespace FSL {

		// FSL data types
		//
		typedef float real; // real = float or double (double requires double RAM!)

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