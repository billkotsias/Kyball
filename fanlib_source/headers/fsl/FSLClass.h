// Fantasia Scripting Language - Class instance

#pragma once
#ifndef FANLIB_FSLClass_H
#define FANLIB_FSLClass_H

#include <deque>
#include <fsl\FSLTypes.h>
#include <fsl\FSLObject.h>
#include <fsl\FSLClassDefPtr.h>

namespace FANLib {

	class FSLClassDefinition;
	class FSLString;
	class FSLArray;
	class FastStr;

	class FSLClass : public FSLObject {
	protected:

		std::deque<FSL::real> data; /// 'real' is enough to contain int, real and pointer data

		void gcClass(); /// empties <data>
		virtual void gc();

		// inline internal service function for generic data access
		// => fstr = variable name
		//	  varType = expected variable type
		//	  errorStr = 'FastStr' error string in case of variable type disagreement
		inline void* get(const FastStr&, const FSL::VAR_TYPE, const FastStr*) const;

		// inline internal service function for changing value of an existing variable
		// => fstr = variable name
		//	  varType = expected variable type
		//	  errorStr = 'FastStr' error string in case of variable type disagreement
		virtual inline int change(const FastStr&, const FSL::VAR_TYPE, const FastStr*);
		virtual ~FSLClass();

	private:

		/// error strings (defined in FSLClassMessages.h)
		static const char* const WRONG_REQ_TYPE[];
		static const char* const WRONG_DEST_TYPE[];
		static const char* const WRONG_CLASS_DEF[];
		static const FastStr WRONG_TYPE_INT;
		static const FastStr WRONG_TYPE_REAL;
		static const FastStr WRONG_TYPE_STRING;
		static const FastStr WRONG_TYPE_ARRAY;
		static const FastStr WRONG_TYPE_CLASS;

	public:

		FSLClassDefPtr definition;
		
		FSLClass(FSLClassDefinition*);

		// get value of a class variable
		// => fstr = variable name
		// <= variable value, or
		//	  exception if it doesn't exist, or is of different type
		int			getInt		(const FastStr&) const;
		FSL::real	getReal		(const FastStr&) const;
		char*		getCString	(const FastStr&) const;
		FSLClass*	getClass	(const FastStr&) const;
		FSLArray*	getArray	(const FastStr&) const;

		// change value of a class variable
		// => fstr = name of variable to change
		//	  value = new value (of according type)
		// <= exception if :
		//		- destination variable doesn't exist
		//		- destination variable is of different type
		//		- destination variable is a class and has a different definition to the new one
		void changeInt		(const FastStr&, int&);
		void changeReal		(const FastStr&, FSL::real&);
		void changeString	(const FastStr&, FSLString*);
		void changeClass	(const FastStr&, FSLClass*);
		void changeArray	(const FastStr&, FSLArray*);

		friend class FSLClassDefinition;
		friend class FSLParser;
	};
}

#endif