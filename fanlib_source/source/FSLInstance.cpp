#include <fsl\FSLInstance.h>

#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLRuntime.h>

#include <core\Log.h>
#include <FANLibErrors.h>
#include <fsl\FSLInstanceMessages.h>

namespace FANLib {

	FSLInstance::~FSLInstance() {

		/// remove class definitions
		for (std::map<FastStr, FSLClassDefinition*>::iterator it = classDefinitions.begin(); it != classDefinitions.end(); ++it) {
			/// definitions have a pointer to themselves, so we artificially remove it to cause garbage collection
			// was : it->second->decreasePointers(); this won't remove definitions with cyclic dependencies - and not only!
			it->second->forceGC();
			/// now delete definition name
			delete [] it->first.str;
		}
		classDefinitions.clear();

		/// remove enums
		for (std::map<FastStr, FSLEnum*>::iterator it = enums.begin(); it != enums.end(); ++it) {
			delete it->second;
			delete [] it->first.str;
		}
		enums.clear();

		/// everything has been GCed, perform actual GC
		FANLib::FSLRuntime::performGC();
	}

	/// get a class definition
	FSLClassDefinition* FSLInstance::getClassDef(const FANLib::FastStr &fstr) const {
		std::map<FastStr, FSLClassDefinition*>::const_iterator it = classDefinitions.find(fstr);
		if (it == classDefinitions.end()) {
			const FastStr* missing_def[] = {&fstr, 0};
			Log::internalLog(MISSING_DEF, missing_def, true, FANLib::Error::FSL_INSTANCE_DEF_EXISTS);
		}
		return it->second;
	}
	FSLClassDefinition* FSLInstance::getClassDef(const char *str) const {
		return getClassDef(FastStr(str));
	}

	/// get enumeration
	FSLEnum* FSLInstance::getEnum(const FastStr& fstr) const {
		std::map<FastStr, FSLEnum*>::const_iterator it = enums.find(fstr);
		if (it == enums.end()) {
			const FastStr* missing_enum[] = {&fstr, 0};
			Log::internalLog(MISSING_ENUM, missing_enum, true, FANLib::Error::FSL_INSTANCE_ENUM_EXISTS);
		}
		return it->second;
	}
	FSLEnum* FSLInstance::getEnum(const char* str) const {
		return getEnum(FastStr(str));
	}

}