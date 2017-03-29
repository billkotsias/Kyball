#include <core\Utils.h>

namespace FANLib {

	Utils::ios_base_setter Utils::IOS_BASE[] = {std::oct, std::dec, std::hex};

	unsigned int Utils::hash(char* str) {

		unsigned char* p = (unsigned char*)str;
		int charPos = 0;
		unsigned int hash = 0;

		while(*(p++)) {
			hash ^= (*p) << charPos;
			charPos += Utils::CHAR_BITS; /// advance shift by 1 char
			if (charPos >= Utils::MAX_CHAR_SHIFT) charPos -= Utils::MAX_CHAR_SHIFT;
		}
		return hash;
	}

	unsigned int Utils::hash(char* str, int size) {

		unsigned char* p = (unsigned char*)str;
		int charPos = 0;
		unsigned int hash = 0;

		while(--size >= 0) {
			hash ^= (p[size]) << charPos;
			charPos += Utils::CHAR_BITS; /// advance shift by 1 char
			if (charPos >= Utils::MAX_CHAR_SHIFT) charPos -= Utils::MAX_CHAR_SHIFT;
		}
		return hash;
	}
}