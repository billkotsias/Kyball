// FSLParser Messages

#pragma once
#ifndef FANLIB_FSLPARSERMESSAGES_H
#define FANLIB_FSLPARSERMESSAGES_H

namespace FANLib {

	const char* const FSLParser::CLASS_REDEF[] = {"Class '","' redefinition", 0};
	const char* const FSLParser::ENUM_REDEF[] = {"Enumeration '","' redefinition", 0};
	const char* const FSLParser::FILE_IERROR[] = {"Can't open file '","' for input", 0};
	const char* const FSLParser::CLASS_UR_REF[] = {"Unrecognized Class reference value '","'", 0};
	const char* const FSLParser::CLASS_UR_NUM[] = {"Unrecognized numeric reference value '","'", 0};
	const char* const FSLParser::CLASS_DIF_REF[] = {"Class reference '","' has different definition", 0};

	const char* const FSLParser::FILE_ERROR[] = {"Fatal error in file '","' at line ", 0};
	const char* const FSLParser::SUBFILE_ERROR[] = {"Subfile error in file '","' at line ", 0};

	const FastStr FSLParser::UQ_NON_BASIC = FastStr("Only basic types can be unique");
	const FastStr FSLParser::ARRAY_ERROR = FastStr("Array can't contain mixed types");
	const FastStr FSLParser::ARRAY_DIM_ERROR = FastStr("Array has too many dimensions");
	const FastStr FSLParser::ARRAY_DEF_ERROR = FastStr("Definition array has different number of dimensions");
	const FastStr FSLParser::ENUM_START = FastStr("Enumeration must start with '{'");
	const FastStr FSLParser::ABRUPT_EOF = FastStr("Abrupt end of file");
}

#endif