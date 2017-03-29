#include <fsl\FSLPointer.h>
#include <fsl\FSLObject.h>
// error-handling
#ifdef _DEBUG
	#include <core\Log.h>
#endif

namespace FANLib {

	FSLPointer::FSLPointer() : ptr(0) {
	}

	FSLPointer::FSLPointer(FSLObject* pObj) {
		ptr = pObj;
		if (pObj) pObj->increasePointers();
	}

	FSLPointer::~FSLPointer() {
		if (ptr) ptr->decreasePointers();
	}

	void FSLPointer::operator=(FSLObject* pObj) {
		if (pObj) pObj->increasePointers(); /// 1st increase, then decrease, in case ptr == pObj. We don't want self-removal bug.
		if (ptr) ptr->decreasePointers();
		ptr = pObj;
	}

	void FSLPointer::operator=(const FSLPointer& fslPtr) {
		(*this) = fslPtr.ptr; /// call above operator
	}

	bool FSLPointer::operator==(const FSLPointer& fslPtr) const {
		return ptr == fslPtr.ptr;
	}

	bool FSLPointer::operator!=(const FSLPointer& fslPtr) const {
		return ptr != fslPtr.ptr;
	}

	bool FSLPointer::operator==(const FSLObject* obj) const {
		return ptr == obj;
	}

	bool FSLPointer::operator!=(const FSLObject* obj) const {
		return ptr != obj;
	}

}