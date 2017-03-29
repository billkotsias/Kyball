// Fantasia Scripting Language - FSLClassDefinition FSLPointer-specialization

#pragma once
#ifndef FANLIB_FSLClassDefPtr_H
#define FANLIB_FSLClassDefPtr_H

#include <fsl\FSLPointer.h>

namespace FANLib {

	class FSLClassDefinition;

	class FSLClassDefPtr : public FSLPointer {
	public:

		// codec
		FSLClassDefPtr(); /// null pointer
		FSLClassDefPtr(FSLClassDefinition*);
		virtual ~FSLClassDefPtr();

		// getter
		inline FSLClassDefinition* getPtr() { /// cause we <can't> overload <operator => as <non-member function>!
			return (FSLClassDefinition*)ptr;
		};

		inline FSLClassDefinition* operator->() const {
			return (FSLClassDefinition*)ptr;
		}

		// overloaded operators
		void operator=(FSLObject*);			/// produces error
		void operator=(const FSLPointer&);	/// produces error

		void operator=(FSLClassDefinition*);
		void operator=(const FSLClassDefPtr&);

	};
}

#endif