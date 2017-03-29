// Fantasia Scripting Language - Dynamic Parser : ASCII -> C++
// - this class will be split to 'FSLParser' and 'FSLInstance' (parser produces an instance from ascii)
// - OR MAYBE THE FSLINSTANCE IS JUST THE ROOT FSLCLASSDEFINITION ?!?
// - later, there will be an 'FSLBinaryMaker' and 'FSLBinary' (maker produces an encoded binary file from an FSL instance)
// - even more later, there will be a STATIC PARSER, which will produce SUPER-EFFICIENT STATIC (read-only) classes

#pragma once
#ifndef FANLIB_FSLParser_H
#define FANLIB_FSLParser_H

#include <map>
#include <stack>
#include <core\FastStr.h>
#include "FSLTypes.h"

extern _CrtMemState s1, s2, result; /// REMOVE THIS DEBUG SHIT!

namespace FANLib {

	class FSLClass;
	class FSLRootArray;
	class FSLPointer;
	class FSLInstance;
	class FSLClassDefinition;
	class FSLEnum;

	class FSLParser {
	private:

		// error strings (defined in FSLParserMessages.h)
		static const char* const CLASS_REDEF[];
		static const char* const ENUM_REDEF[];
		static const char* const FILE_IERROR[];
		static const char* const CLASS_UR_REF[];
		static const char* const CLASS_UR_NUM[];
		static const char* const CLASS_DIF_REF[];
		static const char* const FILE_ERROR[];
		static const char* const SUBFILE_ERROR[];
		static const FastStr UQ_NON_BASIC;
		static const FastStr ARRAY_ERROR;
		static const FastStr ARRAY_DIM_ERROR;
		static const FastStr ARRAY_DEF_ERROR;
		static const FastStr ABRUPT_EOF;
		static const FastStr ENUM_START;

		// keywords
		static const FastStr ENUM;
		static const FastStr INCLUDE;
		static const FastStr CLASS;
		static const FastStr UNIQUE;
		static const FastStr INT;
		static const FastStr REAL;
		static const FastStr STRING;
		static const FastStr ARRAY;
		static const FastStr OF; /// over-bloated stuff
		static const FastStr CURLY_LEFT;
		static const FastStr CURLY_RIGHT;
		static const FastStr CURLY_CLOSED;
		static const FastStr BRACKET_LEFT;
		static const FastStr BRACKET_RIGHT;
		static const FastStr COM_SLASH2;
		static const FastStr COM_SLASH_STAR;
		static const FastStr COM_STAR_SLASH;
		static const FastStr FSL_TRUE;
		static const FastStr FSL_FALSE;
		static const FastStr FSL_NAN;
		static const FastStr FSL_INFINITE;
		static const FastStr COLONS;
		static const char NLCHARS[];
		static const FastStr NL_CHARS;
		static const FastStr SEPARATORS; /// <OS-specific>

		// Internal service functions :
		// - get next keyword
		// => fstr = 'FastStr' currently being parsed
		//	  key = keyword is returned in this 'FastStr' (key.size == 0 if "end of file")
		//	  ignore = ignore comments?
		//	  calc = calculate key's hash?
		//	  eof = report "end of file" error? (meaning an actual keyword was expected)
		// <= "end of file" status (true if EOF)
		inline bool getNextKey(FastStr&, FastStr&, const bool, const bool, const bool);

		// "Included" files holder
		std::map<FastStr, bool> includedFiles;
		std::deque<FastStr> basePath; /// get a "more absolute" path extracted from current relative filename!
		void getFilePath(FastStr&); /// helping function

		// Current 'FSLInstance' being parsed
		FSLInstance* fslInstance;

		// Current class definition being parsed is held in a stack
		std::deque<FSLClassDefinition*> classDefParsed;
		// Current class instance being parsed is held in a stack
		std::deque<FSLClass*> classInstParsed;

		// internal service functions
		// => filename = file path + name - NOTE : won't check if it's relative or absolute path when deciding whether to reject it or not
		//	  reject = reject file if already added? (true for #include directive, false for #inline <to be implemented> )
		void addFile(FastStr&, bool);
		FSLClassDefinition* newClassDef(FastStr&);

		// get a variable reference, in current and higher level scopes
		// => key1 = variable name that was provided as a variable value
		//	  type = expected variable type : different types will be ignored
		// <= pointer to reference, or
		//	  0 if none found
		void* getVarReference(FastStr&, FSL::VAR_TYPE);

		// parse an enumeration
		// => fstr = source text (cursor points after '{')
		FSLEnum* parseEnum(FastStr&);

		// -------------
		// Class parsing
		// -------------
		// (1) Parse Class variable value (uses 'newClassInstance')
		// => fstr = source text (cursor points after 'key1')
		//	  classDef = class definition
		//	  key1 = the variable value must be externally fed
		//	  err = throw error if no proper value found ?
		// <= new class instance, finalized, or
		//	  class referece, or
		//	  class definition referece, or
		//	  0 if no proper value found
		FSLClass* parseClassValue(FastStr&, FSLClassDefinition*, FastStr&, bool);

		// (2) New Class instance (uses 'parseClassInstance')
		// => fstr = source text
		//	  classDef = class definition
		// <= new class instance, finalized
		FSLClass* newClassInstance(FastStr&, FSLClassDefinition*);

		// (3) Parse Class specification
		// => source = FastStr being parsed
		//	  fslClass = class instance getting new values
		void parseClassInstance(FastStr&, FSLClass*); /// this function could be made public..?

		// Parse Array specification
		void parseArray(FastStr&, FSLRootArray*); /// this function could be made public..?

	public:

		FSLParser();
		virtual ~FSLParser();

		// clear all memory held by last file parsing
		void clear();

		// new parse procedure : parse file from disk
		// => filename = file name to parse
		// <= pointer to root class/definition
		FSLInstance* parseFile(const char*) ;

		// get last 'FSLInstance' created from parsing
		FSLInstance* getFSLInstance() { return fslInstance; };

		// new parse procedure : parse a 'FastStr'
		// => source = 'FastStr' to parse
		// <= 'FSLInstance' is updated by the parsed data
		void parseMemory(FastStr&);

		// runtime expression evaluator
		// - very simple for now : string -> number
		// - no calculations
		// - supports enumerations
		FSL::real parseNumericExpression(FastStr&); /// should this function remain public..?
	};
}

#endif