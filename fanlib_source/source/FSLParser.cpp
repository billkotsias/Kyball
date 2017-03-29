#include <fsl\FSLParser.h>

#include <fsl\FSLArray.h>
#include <fsl\FSLRootArray.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLString.h>
#include <fsl\FSLPointer.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>
#include <fstream>
#include <core\Utils.h>
// error-handling
#include <core\Log.h>
#include <FANLibErrors.h>
#include <fsl\FSLParserMessages.h>
#include <math\Math.h>

namespace FANLib {

	// keywords
	const FastStr FSLParser::ARRAY("array", true);
	const FastStr FSLParser::CLASS("class", true);
	const FastStr FSLParser::INCLUDE("#include", true);
	const FastStr FSLParser::ENUM("enum", true);
	const FastStr FSLParser::INT("int", true);
	const FastStr FSLParser::OF("of", true);
	const FastStr FSLParser::REAL("real", true);
	const FastStr FSLParser::STRING("string", true);
	const FastStr FSLParser::UNIQUE("unique", true);
	const FastStr FSLParser::CURLY_LEFT("{", true);
	const FastStr FSLParser::CURLY_RIGHT("}", true);
	const FastStr FSLParser::CURLY_CLOSED("{}", true); /// NOTE : this is different to '{ }'
	const FastStr FSLParser::BRACKET_LEFT("[", true);
	const FastStr FSLParser::BRACKET_RIGHT("]", true);
	const FastStr FSLParser::COM_SLASH2("//", true);
	const FastStr FSLParser::COM_SLASH_STAR("/*", true);
	const FastStr FSLParser::COM_STAR_SLASH("*/", true);
	const FastStr FSLParser::FSL_TRUE("true", true);
	const FastStr FSLParser::FSL_FALSE("false", true);
	const FastStr FSLParser::FSL_NAN("NaN", true);
	const FastStr FSLParser::FSL_INFINITE("oo", true);
	const FastStr FSLParser::COLONS("::", true);
	const char FSLParser::NLCHARS[] = {10, 13};
	const FastStr FSLParser::NL_CHARS(NLCHARS, 2, true);
	const FastStr FSLParser::SEPARATORS("\\/"); /// directory/folder separators (OS-specific), assumed to be 1 char long : if they are not, change OS :-) : < \ / >

	FSLParser::FSLParser() {
	}

	FSLParser::~FSLParser() {
		clear();
	}

	void FSLParser::clear() {
		/// clear included-files holder
		for (std::map<FastStr, bool>::iterator it = includedFiles.begin(); it != includedFiles.end(); ++it) {
			delete [] it->first.str;
		}
		includedFiles.clear();
	}

	FSLInstance* FSLParser::parseFile(const char* filename) {

		/* initialize parsing */;
		clear();
		fslInstance = new FSLInstance(); /// this will be filled with data from the parsed file
		classDefParsed.push_back(newClassDef(FastStr(""))); /// root class has empty ("") id

		/* get base filepath */;
		FastStr path(filename, false);
		getFilePath(path);
		basePath.push_back(path); /// add path in base-paths

		/* add main source file */;
		addFile(FastStr((char*)(filename + path.size)), true); /// remove <absolute> path from file, will be added by base-paths

		/* end parsing */;
		basePath.pop_back(); /// remove path from base-paths; <note> : path.str is not deleted as it's an outsider!
		classDefParsed.pop_back();
		return fslInstance;
	}

	bool FSLParser::getNextKey(FastStr& fstr, FastStr& key, const bool ignore, const bool calc, const bool eof) {

		/// ignore comments?
		while (true) {

			int reset = fstr.cursor;
			fstr.getNextSubstring(key);
			if (key.size == 0 && fstr.cursor == fstr.size) {
				if (eof) Log::internalLog(&ABRUPT_EOF, true, Error::FSL_PARSER_ABRUPT_EOF);
				return true; /// (<EOF>)
			}

			if (ignore) {
				FastStr comment(key.str, 2, true); /// check if it's <//> or </*>
				int commentEndPos;

				if (comment == COM_SLASH2) { /// <//>
					commentEndPos = fstr.charPosition(NL_CHARS, reset + 2); /// get position of the 1st char after this <//> that indicates a <newline>
					fstr.cursor = commentEndPos + 1; /// move cursor after <newline> char

				} else if (comment == COM_SLASH_STAR) { /// </*>
					commentEndPos = fstr.substringPosition(COM_STAR_SLASH, reset + 2); /// get position of the 1st <*/> after this </*>
					fstr.cursor = commentEndPos + 2; /// move cursor after <*/>

				} else {
					break; /// no comment indication, break loop
				}

				if (commentEndPos < 0) { /// comment end not found till EOF
					fstr.cursor = fstr.size; /// next substring will be empty
				}

				continue;
			}

			break; /// don't care about comments
		}

		/// hash?
		if (calc) key.calculateHash();
		//Log::internalLog(&key, true, 0, -1);

		return false; /// not EOF
	}

	FSLClassDefinition* FSLParser::newClassDef(FastStr &fstr) {
		FSLClassDefinition* classDef;

		std::map<FastStr, FSLClassDefinition*>::iterator it = fslInstance->classDefinitions.find(fstr);
		if (it == fslInstance->classDefinitions.end()) {
			classDef = new FSLClassDefinition();
			fstr.str = fstr.toCString(); /// optimization in order not to recalculate hash
			fslInstance->classDefinitions[fstr] = classDef; /// this is where the definition name is re-created (<to be deleted later>)
		} else {
			/// don't produce an error if it's declaration and not redefinition (checked later)
			classDef = it->second;
		}

		return classDef;
	}

	void FSLParser::getFilePath(FANLib::FastStr &fstr) {
		int lastSep = fstr.reverseCharPosition(SEPARATORS);
		fstr.size = lastSep + 1; /// include separator in base filepath, note : separator is assumed to be 1 char long! If it's not, change OS :-)
	}

	void FSLParser::addFile(FastStr &fpath, bool reject) {

		/// store filename to prevent adding it a second time
		/// NOTE : can't tell when a <relative path> == <absolute path>, will treat as different files and may cause errors
		if (reject) {
			if (includedFiles.find(fpath) != includedFiles.end()) return; /// been there
			fpath.str = fpath.toCString(); /// optimization
			includedFiles[fpath] = true;
		}

		/* load file in memory */;
		/// separate filename from filepath
		getFilePath(fpath); /// fpath = <filepath>
		FastStr fname((char*)(fpath.str + fpath.size)); /// fname = <filename>
		/// create new path
		int newPathSize = basePath.back().size + fpath.size; /// new filepath's size (without ending null)
		FastStr newPath(new char[newPathSize + 1], (unsigned int)newPathSize); /// + null
		basePath.back().copyString(newPath.str, false);
		fpath.copyString(newPath.str + basePath.back().size, true); /// newPath = old + new basePath
		basePath.push_back(newPath); /// add new path in base-paths
		/// create extended filename
		char* filename = new char[newPathSize + fname.size + 1];
		basePath.back().copyString(filename, false);
		fname.copyString(filename + basePath.back().size, true); /// filename = old + new basePath + filename
		/// attempt to load file
		std::ifstream streamfile(filename, std::ios::in|std::ios::binary|std::ios::ate); /// open file as binary and move stream pointer to end
		if (!streamfile.is_open()) {
			const FastStr* file_ierror[] = {&FastStr(filename), 0};
			Log::internalLog(FSLParser::FILE_IERROR, file_ierror, true, FANLib::Error::FSL_PARSER_FILE_IERROR);
		}
		delete [] filename; /// filename is no more needed
		std::ifstream::pos_type filesize = streamfile.tellg(); /// get file size
		char* file = new char [filesize]; /// assign enough memory
		streamfile.seekg(0, std::ios::beg);
		streamfile.read(file, filesize);
		streamfile.close();
		FastStr source(file, (unsigned int)filesize, false); /// file is parsed through "source" 'FastStr'

		try {
			parseMemory(source);
		} catch (int errorCode) {
			std::string lineStr = Utils::toString(source.calculateLine(source.cursor - 1) + 1);
			FastStr line(lineStr.c_str(), false);
			const FastStr* fstr[] = {&fname, &line, 0};
		    delete [] file; /// be as polite as possible with memory consumption
			if (errorCode == Error::FSL_PARSER_SUBFILE_ERROR) {
				Log::internalLog(FSLParser::SUBFILE_ERROR, fstr, true, Error::FSL_PARSER_SUBFILE_ERROR);
			} else {
				/// TODO : run <shell command> to open file in <external editor> at error line (<notepad++> supports this)
				Log::internalLog(FSLParser::FILE_ERROR, fstr, true, Error::FSL_PARSER_SUBFILE_ERROR);
			}
		}

		/* end of this file parsing */;
		delete [] basePath.back().str;
		basePath.pop_back();

		/* remove file from memory */;
	    delete [] file;
	}

	FSLEnum* FSLParser::parseEnum(FANLib::FastStr& fstr) {
		FSLEnum* fslEnum = new FSLEnum();
		FastStr key1;
		int enumCount = 0; /// for each enum, count starts at 0
		do {
			getNextKey(fstr, key1, true, true, true); /// allow comments among enumeration values
			if (key1 == FSLParser::CURLY_RIGHT) break;
			key1.str = key1.toCString(); /// optimization for hash
			fslEnum->enums[key1] = enumCount++;
		} while (true);
		return fslEnum;
	}

	// main parsing loop
	void FSLParser::parseMemory(FANLib::FastStr &source) {

		/* start parsing */;
		FastStr key1;
		FastStr key2;

		while (true) {

			/// 1st keyword
			/// - '#include' ("filename")
			/// - 'class' ("ClassName" {...})
			/// - '}' : end of current class definition
			/// - 'enum' ("EnumName" {...})
			/// - 'unique' (type "VarName" "VarValue")
			/// - if none of the above, assume 1st keyword as "ClassName" or type

			getNextKey(source, key1, true, true, false); /// <ignore>, <calc>, <eof>
			if (key1.size == 0) break; /// end of file parsing

			/* new enumeration */;
			if (key1 == FSLParser::ENUM) {
				getNextKey(source, key2, false, true , true); /// key2 = enumeration name
				getNextKey(source, key1, false, true, true); /// must start with a '{'
				if (key1 != FSLParser::CURLY_LEFT)
					Log::internalLog(&ENUM_START, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				std::map<FastStr, FSLEnum*>::iterator it = fslInstance->enums.find(key2);
				if (it == fslInstance->enums.end()) {
					key2.str = key2.toCString(); /// optimazation, in order not to recalculate hash
					fslInstance->enums[key2] = parseEnum(source);
				} else {
					const FastStr* enum_redef[] = {&key2, 0};
					Log::internalLog(ENUM_REDEF, enum_redef, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				}

				continue;
			}

			/* add whole new file (reject if already added) */;
			if (key1 == FSLParser::INCLUDE) {

				getNextKey(source, key1, false, true, true); /// filename to be included
				addFile(key1, true);
				//char* newFileName = key1.toCString();
				//addFile(newFileName, true);
				//delete [] newFileName; /// no longer needed : delete immediately
				continue; /// since there's a <continue> here, it doesn't have to be an <if...else if> statement
			}
			
			/* add new class definition */;
			if (key1 == FSLParser::CLASS) {

				getNextKey(source, key1, false, true, true); /// key1 = name of class definition
				FSLClassDefinition* classDef = newClassDef(key1); /// add class definition (no error if already declared)

				int resetCursor = source.cursor;
				getNextKey(source, key2, false, true, false); /// key2 = check if this is a declaration (no '{') or definition (next substring is '{')
				if (key2 == FSLParser::CURLY_LEFT) {
					/// definition
					if (classDef->defined()) { /// produce <redefinition error>
						const FastStr* class_redef[] = {&key1, 0};
						Log::internalLog(CLASS_REDEF, class_redef, true, FANLib::Error::FSL_PARSER_CLASS_REDEF);
					}
					classDefParsed.push_back(classDef);
				} else {
					/// declaration, still parsing previous class definition
					source.cursor = resetCursor;
				}

				continue;
			}
			
			/* end of current class definition */;
			if (key1 == FSLParser::CURLY_RIGHT) {

					classDefParsed.pop_back();
					if (!classDefParsed.size()) return; /// <end> of file parsing before end of file, allowed for user-debugging
					continue;

			}

			/* unique ? (for int, real or string) */;
			bool unique = false;
			if (key1 == FSLParser::UNIQUE) {
				unique = true;					/// 'unique' registered
				getNextKey(source, key1, false, true, true); /// key1 should now be "ClassName" or type
			}

														 /// key1 = "ClassName" or type
			getNextKey(source, key2, false, true, true); /// key2 = variable name

			if (key1 == FSLParser::INT) {

				if (unique) {
					classDefParsed.back()->newVariable(key2, FSL::UQ_INT, (int)0); /// no default value allowed for unique ints
				} else {
					getNextKey(source, key1, false, false, true); /// key1 = variable value expression
					classDefParsed.back()->newVariable(key2, FSL::INT, (int)(parseNumericExpression(key1)));
				}
				continue;
			}

			if (key1 == FSLParser::REAL) {

				if (unique) {
					classDefParsed.back()->newVariable(key2, FSL::UQ_REAL, (FSL::real)0); /// no default value allowed for unique reals
				} else {
					getNextKey(source, key1, false, false, true); /// key1 = variable value expression
					classDefParsed.back()->newVariable(key2, FSL::REAL, parseNumericExpression(key1));
				}
				continue;
			}

			if (key1 == FSLParser::STRING) {

				if (unique) {
					classDefParsed.back()->newVariable(key2, FSL::UQ_STRING, (char*)0); /// no default value allowed for unique strings
				} else {
					getNextKey(source, key1, false, false, true); /// key1 = variable value expression
					/// <increase pointers of FSLString !!!>
					FSLString* str = new FSLString(key1.toCString());
					str->increasePointers();
					classDefParsed.back()->newVariable(key2, FSL::STRING, str);
				}
				continue;
			}

			if (unique) { /// no unique allowed from here on
				Log::internalLog(&UQ_NON_BASIC, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
			}

			if (key1 == FSLParser::ARRAY) {
				FSLRootArray* newArray = new FSLRootArray();

				do {
					getNextKey(source, key1, false, true, true); /// key1 = variable value expression
				} while (key1 == FSLParser::OF); /// jump any 'of's

				/// get array type
				if (key1 == FSLParser::INT)		{ newArray->deepType = FSL::INT;	} else
				if (key1 == FSLParser::REAL)	{ newArray->deepType = FSL::REAL;	} else
				if (key1 == FSLParser::STRING)	{ newArray->deepType = FSL::STRING;	} else {
					/// array is of type CLASS
					newArray->definition = fslInstance->classDefinitions.find(key1)->second;
					newArray->deepType = FSL::CLASS;
				}
				getNextKey(source, key1, false, true, true); /// key1 = array name
				parseArray(source, newArray);	/// parse array data
				newArray->increasePointers();	/// add array in current class definition
				classDefParsed.back()->newVariable(key1, FSL::ARRAY, newArray);
				continue;
			}

			/* it was none of the base types, search for class name */;
			/// check if variables points to 1) definition, 2) reference or 3) new Class instance
			FSLClassDefinition* classDef = fslInstance->classDefinitions.find(key1)->second;

			int resetCursor = source.cursor; /// allow <nothing> as variable value (neither reference, nor new instance)
			getNextKey(source, key1, false, true, false); /// key1 = variable value
			FSLClass* classPtr = parseClassValue(source, classDef, key1, false);

			if (classPtr == 0) { /// allow <nothing> as variable value (neither reference, nor new instance)
				source.cursor = resetCursor; /// go back to re-parse last keyword
				classPtr = classDef;
			}
			/// <increase pointers of classPtr !!!>
			classPtr->increasePointers();
			classDefParsed.back()->newVariable(key2, FSL::CLASS, classPtr);

		}
	}

	FSL::real FSLParser::parseNumericExpression(FANLib::FastStr &fstr) {
		if (fstr.size <= 0) return 0; /// this should never occur, but just in case...

		/// TODO :
		/// - use the penultimate function 'getVarReference()'
		/// - recognize separate strings, evaluate each, then perform arithmetic operation between them

		/// check if this is an enumeration reference
		/// - must contain the substring '::', eg. "enumName::enumValue"
		int colon = fstr.substringPosition(COLONS);
		if (colon >= 0) {
			FSLEnum* enu = fslInstance->getEnum(FastStr(fstr.str, (unsigned int)colon));
			FastStr enumValue((fstr.str + colon + 2), (unsigned int)(fstr.size - colon - 2));
			return (FSL::real)enu->getValue(enumValue);
		}

		/// check if there is a radix prefix
		char firstChar = fstr.str[0];
		Utils::STRING_BASE iosBase;

		if (firstChar == "%"[0]) { /// <binary>
			iosBase = Utils::BINARY;
			fstr.trim(1); /// remove prefix before conversion

		} else if (firstChar == "0"[0] && fstr.size >= 3 && fstr.str[1] == "x"[0]) { /// <HEX> : 'fstr' must have at least 3 chars, eg. "0x1"
			iosBase = Utils::HEX;
			fstr.trim(2); /// remove prefix before conversion

		} else {
			iosBase = Utils::DECIMAL; /// <default> base
		}

		FSL::real value; /// return this
		if (Utils::fromString(value, fstr.str, iosBase))
		{	/// conversion error occured
			fstr.calculateHash();
			if (fstr == FSL_TRUE) return true;
			if (fstr == FSL_FALSE) return false;
			if (fstr == FSL_NAN) return NaNf; /// or NaN <!>
			if (fstr == FSL_INFINITE) return Infinitef; /// <!>
			/// nothing of the above
			const FastStr* class_ur_num[] = {&fstr, 0};
			Log::internalLog(CLASS_UR_NUM, class_ur_num, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
		}
		return value;
	}

	void* FSLParser::getVarReference(FANLib::FastStr &key1, FANLib::FSL::VAR_TYPE type) {
		/// 1) check parsed class instances
		for (int i = classInstParsed.size() - 1; i >= 0; --i) {
			FSLClass* fslClass = classInstParsed.at(i);
			FSLClassDefinition* classDef = fslClass->definition.getPtr();
			std::map<FastStr, int>* var = &(classDef->var);
			std::map<FastStr, int>::const_iterator it = var->find(key1); /// check if var name is in this scope

			/// reject scope if variable name not found, or variable is of different type
			if (it == var->end()) continue;
			int pos = it->second;
			if (classDef->type.at(pos) != type) continue;
			return (void*)&(fslClass->data.at(pos));
		}

		/// 2) check parsed class definition
		for (int i = classDefParsed.size() - 1; i >= 0; --i) {
			FSLClassDefinition* classDef = classDefParsed.at(i);
			std::map<FastStr, int>* var = &(classDef->var);
			std::map<FastStr, int>::const_iterator it = var->find(key1); /// check if var name is in this scope

			/// reject scope if variable name not found, or variable is of different type
			if (it == var->end()) continue;
			int pos = it->second;
			if (classDef->type.at(pos) != type) continue;
			return (void*)&(classDef->data.at(pos));
		}
		
		return 0;

	}

	// -------------
	// Class parsing
	// -------------
	FSLClass* FSLParser::parseClassValue(FANLib::FastStr &fstr, FANLib::FSLClassDefinition *classDef, FANLib::FastStr& key1, bool err) {
		FSLClass* classPtr;

		if (key1 == FSLParser::CURLY_LEFT) {
			classPtr = newClassInstance(fstr, classDef);

		} else if (key1 == FSLParser::CURLY_CLOSED)	{
			classPtr = classDef;
		
		} else {
			// check for variable reference in this-and-higher level scopes
			FSLClass** referencePointer = (FSLClass**)getVarReference(key1, FSL::CLASS);
			if (referencePointer == 0) { /// no reference found
				if (err) {
					const FastStr* class_ur_ref[] = {&key1, 0};
					Log::internalLog(CLASS_UR_REF, class_ur_ref, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				}
				classPtr = 0; /// if none found, return 0

			} else {
				classPtr = *referencePointer;
				if (classPtr->definition != classDef) { /// check if reference class has different definition
					const FastStr* class_dif_ref[] = {&key1, 0};
					Log::internalLog(CLASS_DIF_REF, class_dif_ref, true, FANLib::Error::FSL_PARSER_WRONG_DEF);
				}
			}
			
		}

		return classPtr;
	}

	FSLClass* FSLParser::newClassInstance(FANLib::FastStr &fstr, FANLib::FSLClassDefinition *classDef) {
		FSLClass* classPtr = classDef->createInstance();

		classInstParsed.push_back(classPtr); /// keep variable-scope levels in this stack when searching for a <reference-variable value>
		parseClassInstance(fstr, classPtr);
		classInstParsed.pop_back(); /// end of this class's scope

		classDef->finalize(classPtr); /// check for <unique> values / <increase> pointers

		return classPtr;
	}

	void FSLParser::parseClassInstance(FastStr &fstr, FSLClass* fslClass) {
		/// parse text in pairs : variable name - variable value
		/// case 1 : if variable is a class, the "value" is a '{' and a new 'parseClassInstance' will be issued
		FastStr key1;
		FSLClassDefinition* definition = (FSLClassDefinition*)fslClass->definition.getPtr();

		do {
			getNextKey(fstr, key1, true, true, true); /// key1 = variable name
			if (key1 == FSLParser::CURLY_RIGHT) return; /// end of class specification

			int pos = definition->getPosition(key1);		/// variable position
			unsigned int type = definition->getType(pos);	/// variable type
			switch (type) {

				case FSL::INT: {
					getNextKey(fstr, key1, false, false, true); /// key1 = variable value
					int valueInt = (int)parseNumericExpression(key1);
					fslClass->data.at(pos) = *(FSL::real*)&valueInt;
					break; }
				case FSL::REAL: {
					getNextKey(fstr, key1, false, false, true); /// key1 = variable value
					FSL::real valueReal = parseNumericExpression(key1);
					fslClass->data.at(pos) = *(FSL::real*)&valueReal;
					break; }
				case FSL::STRING: {
					getNextKey(fstr, key1, false, false, true); /// key1 = variable value
					FSLString* str = new FSLString(key1.toCString());
					fslClass->data.at(pos) = *(FSL::real*)&str; /// pointers of 'str' will be increased when "finalized"
					break; }
				case FSL::ARRAY: {
					FSLRootArray* newArray = new FSLRootArray((*(FSLRootArray**)&(fslClass->data.at(pos))));
					parseArray(fstr, newArray);
					fslClass->data.at(pos) = *(FSL::real*)&newArray; /// pointers of 'newArray' will be increased when "finalized"
					break; }
				case FSL::CLASS: {
					getNextKey(fstr, key1, false, true, true); /// key1 = variable value
					FSLClassDefinition* classDef = (*(FSLClass**)&(fslClass->data.at(pos)))->definition.getPtr();
					FSLClass* newClass = parseClassValue(fstr, classDef, key1, true);
					fslClass->data.at(pos) = *(FSL::real*)&newClass; /// pointers of 'newClass' will be increased when "finalized"
					break; }
			}

		} while (true);

	}

	// -------------
	// Array parsing
	// -------------
	void FSLParser::parseArray(FANLib::FastStr &fstr, FSLRootArray *rootArray) {

		FastStr key1;
		std::stack<FSLArray*> subarrays; /// back -> currently parsed array

		int reset;
		/// is this a definition or a declaration?
		reset = fstr.cursor;
		getNextKey(fstr, key1, false, true, true); /// check for initial '['
		if (key1 != FSLParser::BRACKET_LEFT) {
			/// <TODO> : allow array references as definitions (not just direct [ ... ] values) <!!!>
			//FSLArray** referencePointer = (FSLArray**)getVarReference(key1, FSL::ARRAY);
			//if (referencePointer == 0) { /// no reference found
				fstr.cursor = reset;
				if (rootArray->dimensions == 0) return; /// this is a declaration, there will be no initial restrictions and error-checking for its instances
				Log::internalLog(&ARRAY_DEF_ERROR, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR); /// this should be an instantiation --> error
			//} else {

			//}
		}

		/// we have already count one '[' which means at least 1 dimension array
		int dimensions = 1;
		reset = fstr.cursor; /// after the dim-count below, parsing will restart AFTER the 1st bracket
		subarrays.push(rootArray); /// put default array in stack

		/// count all dimensions
		while (true) {
			getNextKey(fstr, key1, true, true, true); /// count '[', allowing comments in-between
			if (key1 != FSLParser::BRACKET_LEFT) break;
			++dimensions;
		}
		fstr.cursor = reset; /// as I said, start after the 1st '['

		if (rootArray->dimensions != 0) {
			if (rootArray->dimensions != dimensions) {
				Log::internalLog(&ARRAY_DEF_ERROR, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
			}
		} else {
			rootArray->dimensions = dimensions; /// save our findings
		}
		dimensions = 1;

		/// start of array parsing...

		getNextKey(fstr, key1, true, true, true); /// allow comments in-between

		while (true) {

			FSLArray* subArray;

			/// '[' increases current dimension, ']' decreases
			/// if we have a key that's not a bracket, then it's an error unless we are at maximum dimension allowed
			/// if current dimension exceeds maximum allowed, it's an error

			if (key1 == FSLParser::BRACKET_LEFT) {

				/// there are subarrays
				if (subarrays.top()->setType(FSL::ARRAY)) { /// the last created array contains arrays
					Log::internalLog(&ARRAY_ERROR, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				}
				subArray = new FSLArray();
				subArray->increasePointers(); /// added once, below
				subarrays.top()->newValue(FSL::ARRAY, subArray);
				subarrays.push(subArray);

				++dimensions;
				if (dimensions > rootArray->dimensions) {
					Log::internalLog(&ARRAY_DIM_ERROR, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				}

				getNextKey(fstr, key1, true, true, true); /// allow comments in-between
				continue;
			}

			if (key1 == FSLParser::BRACKET_RIGHT) {

				--dimensions;
				subarrays.pop(); /// farewell, my beloved array
				if (dimensions == 0) return; /// end of parsing

				getNextKey(fstr, key1, true, true, true); /// allow comments in-between
				continue;
			}

			/// parsing of array values has started right here
			while (key1 != FSLParser::BRACKET_RIGHT) {

				subArray = subarrays.top();
				if (subArray->setType(rootArray->deepType)) { /// we are parsing values, so this array contains values
					Log::internalLog(&ARRAY_ERROR, true, FANLib::Error::FSL_PARSER_SYNTAX_ERROR);
				}

				if (rootArray->deepType == FSL::INT)	{
					do {
						subArray->newValue(FSL::INT, (int)parseNumericExpression(key1));
						getNextKey(fstr, key1, false, true, true); /// key1 = value
					} while (key1 != FSLParser::BRACKET_RIGHT);
					continue;
				}

				if (rootArray->deepType == FSL::REAL)	{
					do {
						subArray->newValue(FSL::REAL, parseNumericExpression(key1));
						getNextKey(fstr, key1, false, true, true); /// key1 = value
					} while (key1 != FSLParser::BRACKET_RIGHT);
					continue;
				}

				if (rootArray->deepType == FSL::STRING)	{
					do {
						FSLString* str = new FSLString(key1.toCString());
						str->increasePointers(); /// added below
						subArray->newValue(FSL::STRING, str);
						getNextKey(fstr, key1, false, true, true); /// key1 = value
					} while (key1 != FSLParser::BRACKET_RIGHT);
					continue;
				}

				if (rootArray->deepType == FSL::CLASS)	{
					do {
						FSLClassDefinition* classDef = rootArray->definition;
						FSLClass* classPtr = parseClassValue(fstr, classDef, key1, true);
						classPtr->increasePointers(); /// <increase pointers of classPtr !!!>
						subArray->newValue(FSL::CLASS, classPtr);
						getNextKey(fstr, key1, true, true, true); /// key1 = value
					} while (key1 != FSLParser::BRACKET_RIGHT);
					continue;
				}

			} /// while (key1 != FSLParser::BRACKET_RIGHT)

		} /// while(true)

	} /// ::parseArray()

}