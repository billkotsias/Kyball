#include <fsl\FSLEnum.h>

#include <fsl\FSLEnumMessages.h>
#include <core\Log.h>
#include <FANLibErrors.h>

namespace FANLib {

	FSLEnum::~FSLEnum() {
		std::map<FastStr, int>::iterator it;
		for (it = enums.begin(); it != enums.end(); ++it) {
			delete [] it->first.str;
		}
	}

	unsigned int FSLEnum::getSize() {
		return enums.size();
	}

	int FSLEnum::getValue(const FANLib::FastStr &fstr) const {

		std::map<FastStr, int>::const_iterator it = enums.find(fstr);
		if (it == enums.end()) {
			const FastStr* value_not_found[] = {&fstr, 0};
			Log::internalLog(VALUE_NOT_FOUND, value_not_found, true, FANLib::Error::FSL_ENUM_VALUE_NOT_FOUND);
		}

		return it->second;
	}

}