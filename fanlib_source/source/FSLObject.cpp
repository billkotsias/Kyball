#include <fsl\FSLObject.h>

#include <fsl\FSLRuntime.h>

namespace FANLib {

	FSLObject::FSLObject() : pointers(0) {
	}

	FSLObject::~FSLObject() {
	}

	void FSLObject::increasePointers() {
		++pointers;
	}

	void FSLObject::decreasePointers() {
		if (pointers-- == 1) {
			FSLRuntime::addToGC(this);
		}
	}

	void FSLObject::forceGC() {
		pointers = 0;
		FSLRuntime::addToGC(this);
	}
}