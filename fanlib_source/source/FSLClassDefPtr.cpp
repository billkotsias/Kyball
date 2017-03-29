#include <fsl\FSLClassDefPtr.h>

#include <fsl\FSLClassDefinition.h>
// error-handling
#ifdef _DEBUG
	#include <FANLibErrors.h>
	#include <core\Log.h>
#endif

namespace FANLib {

	FSLClassDefPtr::FSLClassDefPtr() : FSLPointer() {
	}

	FSLClassDefPtr::FSLClassDefPtr(FSLClassDefinition* pObj) : FSLPointer((FSLObject*)pObj) {
	}

	FSLClassDefPtr::~FSLClassDefPtr() {
	}

// =========================
/// NOTE : In <DEBUG MODE>, assigning FSLObject or FSLPointer to FSLClassDefPtr will produce a <runtime> error !
///		 : In <RELEASE MODE>, it will produce a <compile-time> error !
#ifdef _DEBUG
	void FSLClassDefPtr::operator=(FSLObject* pObj) {
		Log::internalLog("Cannot assign FSLObject to FSLClassDefPtr", true, FANLib::Error::FSL_CLASSDEFPTR_ASSIGN);
	}

	void FSLClassDefPtr::operator=(const FSLPointer& fslPtr) {
		Log::internalLog("Cannot assign FSLPointer to FSLClassDefPtr", true, FANLib::Error::FSL_CLASSDEFPTR_ASSIGN);
	}
#endif
// =========================

	void FSLClassDefPtr::operator=(FSLClassDefinition* pObj) {
		if (pObj) pObj->increasePointers(); /// 1st increase, then decrease, in case ptr == pObj. We don't want self-removal bug.
		if (ptr) ptr->decreasePointers();
		ptr = pObj;
	}

	void FSLClassDefPtr::operator=(const FSLClassDefPtr& fslPtr) {
		(*this) = (FSLClassDefinition*)fslPtr.ptr; /// call above operator
	}

}