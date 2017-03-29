// Utils : several tools for every-day use
#pragma once
#ifndef FANLIB_Utils_H
#define FANLIB_Utils_H

#include <sstream>
#include <string>
#include <bitset>

namespace FANLib {

	class Utils {
	public:
		// constructor, throws exception. Class is not designed for instantiation
		Utils() { throw "Utils is not meant to be instantiated"; };

		// <template>
		// convert any class to std::string (including numbers)
		// => object = object to be converted
		// <= object as std::string
		template <class Class>
		static std::string toString(const Class& object) {
			std::stringstream ss;
			ss << object; /* we will know at compile time if this operation is supported */;
			return ss.str();
		};

		// <template>
		// convert an std::string to any class (including numbers)
		// => t = class instance to convert std::string into (eg. an 'int' instance)
		//	  s = the string to convert
		//	  f = base conversion, should be any of : std::dec, std::hex, std::oct
		// <= bool whether conversion failed
		//	  t is updated to the string conversion result
		enum STRING_BASE : signed int {
			BINARY	= -1,
			OCTAL	= 0,
			DECIMAL	= 1,
			HEX		= 2,
		};
		typedef std::ios_base& (*ios_base_setter)( std::ios_base& );
		static ios_base_setter IOS_BASE[];
		template <class Class>
		static bool fromString(Class& t, const std::string& str, STRING_BASE base = DECIMAL) {
			if (base == BINARY) {
				t = (Class)(std::bitset<(sizeof(unsigned long)*8)>(str)).to_ulong();
				return true;
			}
			std::istringstream iss(str);
			return (iss >> IOS_BASE[(int)base] >> t).fail();
		};
		template <class Class>
		static bool fromString(Class& t, const char* str, STRING_BASE base = DECIMAL) {
			return fromString(t, std::string(str), base);
		};

		// <template>
		// find position of a value inside a container
		// - does reverse search
		// => container
		//	  container's last position (starts from this one)
		//	  value to search for
		// <= int = [0 ... last position - 1] if found, -1 if not
		template <class Container, class Value>
		static int rFind(Container& container, int last, Value& value) {
			int i;
			for (i = last - 1; i >= 0; --i) {
				if (container[i] == value) break;
			}
			return i;
		};

		// char size in bits
		static const int CHAR_BITS = sizeof(char) << 3;

		// 'minimum illegal' shifted position of a char (for 32bit systems = 25)
		static const int MAX_CHAR_SHIFT = (sizeof(int) << 3) - CHAR_BITS + 1;

		// produce hash from c-string
		// => str = c-string
		// <= int hash
		static unsigned int hash(char*);

		// produce hash for string with known size
		// => str = string
		//	  size = string size
		// <= int hash
		static unsigned int hash(char*, int);

	};
}

#endif