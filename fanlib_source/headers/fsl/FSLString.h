// Fantasia Scripting Language - FSL String

#pragma once
#ifndef FANLIB_FSLString_H
#define FANLIB_FSLString_H

#include <fsl\FSLObject.h>

namespace FANLib {

	class FSLString : public FSLObject {
	protected:
		virtual ~FSLString();

	private:

		char* str; /// FSLString is in fact an 'FSLObject' wrapper for 'char*'
		// NOTE : string must have been created with 'new', because 'delete []' is called on it

		virtual void gc();

	public:

		virtual void increasePointers();
		virtual void decreasePointers();

		// constructors / destructor
		FSLString(char*);

		char* getCString();

		friend class FSLPointer;
	};
}

#endif