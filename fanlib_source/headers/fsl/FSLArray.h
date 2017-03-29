// Fantasia Scripting Language - Array instance (simplified Class)

#pragma once
#ifndef FANLIB_FSLArray_H
#define FANLIB_FSLArray_H

#include <deque>
#include <fsl\FSLTypes.h>
#include <fsl\FSLObject.h>
// error-handling
#ifdef _DEBUG
	#include <core\Log.h>
	#include <FANLibErrors.h>
#endif

namespace FANLib {

	class FSLClass;
	class FastStr;
	class FSLString;

	class FSLArray : public FSLObject {
	protected:
		std::deque<FSL::real> data;
		FSL::VAR_TYPE type;
		virtual ~FSLArray();

		virtual void gc();

		// inline internal service function for generic data access
		// => pos = variable position in array
		//	  varType = expected variable type
		//	  errorStr = 'FastStr' error string in case of variable type disagreement
		inline void* get(const unsigned int&, const FSL::VAR_TYPE, const FastStr*) const;
		inline void change(const unsigned int&, const FSL::VAR_TYPE, const FastStr*);

	private:

		/// error strings (defined in FSLClassMessages.h)
		static const char* const WRONG_REQ_TYPE[];
		static const char* const WRONG_DEST_TYPE[];
		static const FastStr WRONG_TYPE_INT;
		static const FastStr WRONG_TYPE_REAL;
		static const FastStr WRONG_TYPE_STRING;
		static const FastStr WRONG_TYPE_ARRAY;
		static const FastStr WRONG_TYPE_CLASS;
		static const FastStr WRONG_CLASS_DEF;
		static const FastStr OUT_OF_BOUNDS;
		static const FastStr ARRAY_WRONG_TYPE;


	public:

		FSLArray();

		// get number of array's elements
		inline unsigned int getSize() const { return data.size(); };

		// set array's values type
		// => inType = new type
		// <= returns true (error) if type is already set, and new type is different to it
		bool setType(FSL::VAR_TYPE);
		FSL::VAR_TYPE getType() { return type; };

		// get value of a class variable
		// => pos = variable position in array
		// <= variable value, or
		//	  exception if it doesn't exist, or is of different type
		int			getInt		(const unsigned int&) const;
		FSL::real	getReal		(const unsigned int&) const;
		char*		getCString	(const unsigned int&) const;
		FSLClass*	getClass	(const unsigned int&) const;
		FSLArray*	getArray	(const unsigned int&) const;

		// change value of a class variable
		// => fstr = name of variable to change
		//	  value = new value (of according type)
		// <= exception if :
		//		- destination variable doesn't exist
		//		- destination variable is of different type
		//		- destination variable is a class and has a different definition to the new one
		void changeInt		(const unsigned int&, int&);
		void changeReal		(const unsigned int&, FSL::real&);
		void changeString	(const unsigned int&, FSLString*);
		void changeClass	(const unsigned int&, FSLClass*);
		void changeArray	(const unsigned int&, FSLArray*);

		// add new value in array
		// => type = variable type
		//	  Class p = int, real or FSLObject* : all fit nicely within a 'real'
		template <class Class>
		void newValue(FSL::VAR_TYPE inType, Class p) {
#ifdef _DEBUG
			if (type != inType) {
				Log::internalLog(&ARRAY_WRONG_TYPE, true, FANLib::Error::FSL_ARRAY_WRONG_TYPE);
			}
#endif
			data.push_back(*(FSL::real*)&p);
		};

		friend class FSLParser;
	};
}

#endif