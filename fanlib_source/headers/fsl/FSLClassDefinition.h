// Fantasia Scripting Language - Class definition

#pragma once
#ifndef FANLIB_FSLClassDefinition_H
#define FANLIB_FSLClassDefinition_H

#include <deque>
#include <map>
#include <core\FastStr.h>
#include "FSLClass.h"
#include "FSLTypes.h"
// error-handling
#include <core\Log.h>
#include <FANLibErrors.h>

namespace FANLib {

	class FSLClassDefinition: public FSLClass {
	private:
		std::map<FastStr, int>	var;	/// variable name
		std::deque<char>		type;	/// variable type + boolean bits (constant)

		void gcClassDef(); /// delete memory held for <var> names
		virtual void gc();

		/// error strings (defined in FSLClassMessages.h)
		static const char* const VAR_NAME_EXISTS[];
		static const char* const VAR_NOT_FOUND[];
		static const char* const VAR_IS_DEF[];

		// the functions below are private so as to force minimal error-checking from the user's side (he must know the vars' type in advance)
		bool varExists(const FastStr&) const;		/// <= does this variable exist?
		int getPosition(const FastStr&) const;		/// <= position of given variable in map
		unsigned int getType(const int&) const;		/// <= type of variable at given position
		unsigned int getFullType(const int&) const;	/// <= type of variable at given position, inc. boolean bits (eg. 'unique')

		// create a Class instance from this definition
		FSLClass* createInstance();
		// finalize a newly created instance
		/// - checks unique values
		/// - increases pointers of FSLObjects
		void finalize(FSLClass*);

	protected:
		// override parent's function to prevent changes
		virtual inline int change(const FastStr& fstr, const FSL::VAR_TYPE varType, const FastStr* errorStr) {
			const FastStr* var_is_def[] = {&fstr, 0};
			Log::internalLog(VAR_IS_DEF, var_is_def, true, FANLib::Error::FSL_CLASSDEF_VAR_IS_DEF);
			return -1;
		}

	public:

		FSLClassDefinition();
		virtual ~FSLClassDefinition();

		// add new variable in Class definition
		// => fstr = variable name
		//	  type = variable type
		//	  Class p = int, real or FSLObject* : all fit nicely within a 'real'
		// NOTES :	- throws error if name already exists
		//			- creates a new copy of 'FastStr', so old one may/should be deleted by user
		template <class Class>
		void newVariable(const FastStr &fstr, FSL::VAR_TYPE inType, Class p) {
			if (varExists(fstr)) {
				const FastStr* var_name_exists[] = {&fstr, 0};
				Log::internalLog(VAR_NAME_EXISTS, var_name_exists, true, FANLib::Error::FSL_CLASSDEF_VAR_NAME_EXISTS);
			}
			var[FastStr(fstr.toCString(), (unsigned int)fstr.size)] = data.size(); /// the created (copied) string must be deleted later
			type.push_back((char)inType);
			data.push_back(*(FSL::real*)&p);
		}

		bool defined();	/// is this a declaration or definition?

		friend class FSLClass;
		friend class FSLParser;
	};
}

#endif