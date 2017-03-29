// Fantasia Scripting Language - Pointer to FSLObject

#pragma once
#ifndef FANLIB_FSLPointer_H
#define FANLIB_FSLPointer_H

namespace FANLib {

	class FSLObject;

	class FSLPointer {
	protected:

		FSLObject* ptr;

	public:

		// codec
		FSLPointer(); /// null pointer
		FSLPointer(FSLObject*);
		virtual ~FSLPointer();

		// getter
		inline FSLObject* getPtr() { /// cause we <can't> overload <operator => as <non-member function>!
			return ptr;
		};

		inline FSLObject* operator->() const {
			return ptr;
		}

		// overloaded operators
		void operator=(FSLObject*);
		void operator=(const FSLPointer&);

		bool operator==(const FSLPointer&) const;
		bool operator!=(const FSLPointer&) const;

		bool operator==(const FSLObject*) const;
		bool operator!=(const FSLObject*) const;
	};
}

#endif