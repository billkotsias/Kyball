#include <fsl\FSLRuntime.h>
#include <fsl\FSLObject.h>

#ifdef _DEBUG
	#include <core\Log.h>
#endif

namespace FANLib {

	std::deque<FSLObject*> FSLRuntime::bin;

	void FSLRuntime::addToGC(FANLib::FSLObject* fslPtr) {
#ifdef _DEBUG
		Log::internalLog("Added another object to GC");
#endif
		FSLRuntime::bin.push_back(fslPtr);
	}

	void FSLRuntime::performGC() {
		std::deque<FSLObject*> dead; // garbage collected objects are put here for deletion

		while (bin.size() > 0) {
			bin.front()->gc();
			dead.push_back(bin.front());
			bin.pop_front(); /// hope this doesn't delete the FSLObject itself, but only the pointer to it
		}

#ifdef _DEBUG
		Log::internalLog("GC finished, deleting objects...");
#endif

		while (dead.size()) {
			delete dead.back(); /// delete object
			dead.pop_back();
		}

#ifdef _DEBUG
		Log::internalLog("Deleting also finished!");
#endif
	}

}