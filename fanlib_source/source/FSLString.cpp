#include <fsl\FSLString.h>

#include <fsl\FSLRuntime.h>
#ifdef _DEBUG
	#include <core\Log.h>
	#include <core\FastStr.h>
	#include <core\Utils.h>
#endif

namespace FANLib {

	FSLString::FSLString(char* pStr) : str(pStr) {
	}

	FSLString::~FSLString() {
#ifdef _DEBUG
		//Log::internalLog("FSLString : deleting the remainings of.");
#endif
	}

	char* FSLString::getCString() {
		return str;
	}

	void FSLString::increasePointers() {
#ifdef _DEBUG
		Log::internalLog("FSLString has a pointer added =", false);
		Log::internalLog(str, false, 0, -1);
		Log::internalLog(Utils::toString(pointers).c_str(), true, 0, -1);
#endif
		++pointers;
	}

	void FSLString::decreasePointers() {
#ifdef _DEBUG
		Log::internalLog("FSLString is minus a pointer =", false);
		Log::internalLog(str, false, 0, -1);
		Log::internalLog(Utils::toString(pointers).c_str(), true, 0, -1);
#endif
		if (pointers-- == 1) {
#ifdef _DEBUG
			Log::internalLog("FSLString is going to GC =", false, 0, 0);
			Log::internalLog(str, true, 0, -1);
#endif
			FSLRuntime::addToGC(this);
		}
	}

	void FSLString::gc() {
#ifdef _DEBUG
		Log::internalLog("GCing FSLString", false, 0, 0);
		Log::internalLog(str, true, 0, -1);
#endif
		if (str) delete [] str; /// NOTE : string must have been created with 'new' !!!
	}
}