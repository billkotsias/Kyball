// FSLArray Messages

#pragma once
#ifndef FANLIB_FSLARRAYMESSAGES_H
#define FANLIB_FSLARRAYMESSAGES_H

#include <core\FastStr.h>

namespace FANLib {

	/// FSL messages
	const char* const FSLArray::WRONG_REQ_TYPE[] = {"Requested array variable is not ", 0};
	const char* const FSLArray::WRONG_DEST_TYPE[] = {"Destination array variable is not ", 0};

	const FastStr FSLArray::WRONG_TYPE_INT = FastStr("INT", false);
	const FastStr FSLArray::WRONG_TYPE_REAL = FastStr("REAL", false);
	const FastStr FSLArray::WRONG_TYPE_STRING = FastStr("STRING", false);
	const FastStr FSLArray::WRONG_TYPE_ARRAY = FastStr("ARRAY", false);
	const FastStr FSLArray::WRONG_TYPE_CLASS = FastStr("CLASS", false);

	const FastStr FSLArray::WRONG_CLASS_DEF = FastStr("Destination array variable contains incompatible class", false);
	const FastStr FSLArray::ARRAY_WRONG_TYPE = FastStr("Destination array is of different type", false);
	const FastStr FSLArray::OUT_OF_BOUNDS = FastStr("Variable position is out of array's bounds", false);
}

#endif