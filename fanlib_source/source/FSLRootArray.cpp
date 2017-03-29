#include <fsl\FSLRootArray.h>

namespace FANLib {

	FSLRootArray::FSLRootArray() : deepType(FSL::NULL_TYPE), definition(0), dimensions(0) {
	}

	FSLRootArray::FSLRootArray(FSLRootArray* arr) {
		deepType = arr->deepType;
		definition = arr->definition;
		dimensions = arr->dimensions;
	}

}