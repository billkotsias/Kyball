// Fantasia Scripting Language - Enumeration instance

#pragma once
#ifndef FANLIB_FSLEnum_H
#define FANLIB_FSLEnum_H

#include <map>
#include <core\FastStr.h>

namespace FANLib {

	class FSLEnum {
	public:

		~FSLEnum();
		unsigned int getSize();
		int getValue(const FastStr&) const;

	private:

		std::map<FastStr, int> enums;
		static const char* const VALUE_NOT_FOUND[];

		friend class FSLParser;
	};
}

#endif