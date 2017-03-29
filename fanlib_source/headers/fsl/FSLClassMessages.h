// FSLClass Messages

#pragma once
#ifndef FANLIB_FSLCLASSMESSAGES_H
#define FANLIB_FSLCLASSMESSAGES_H

#include <core\FastStr.h>

namespace FANLib {

	/// FSL messages
	const char* const FSLClass::WRONG_REQ_TYPE[] = {"Requested variable '","' is not ", 0};
	const char* const FSLClass::WRONG_DEST_TYPE[] = {"Destination variable '","' is not ", 0};
	const char* const FSLClass::WRONG_CLASS_DEF[] = {"Destination variable '","' contains incompatible class", 0};

	const FastStr FSLClass::WRONG_TYPE_INT = FastStr("INT", false);
	const FastStr FSLClass::WRONG_TYPE_REAL = FastStr("REAL", false);
	const FastStr FSLClass::WRONG_TYPE_STRING = FastStr("STRING", false);
	const FastStr FSLClass::WRONG_TYPE_ARRAY = FastStr("ARRAY", false);
	const FastStr FSLClass::WRONG_TYPE_CLASS = FastStr("CLASS", false);
}

#endif