// Fast Strings : special purpose fast strings for text manipulation
// They share commmon memory with other strings, so that sub-strings can easily be accessed without making new copies
#pragma once
#ifndef FANLIB_FastStr_H
#define FANLIB_FastStr_H

#include <iostream>

namespace FANLib {

	class FastStr {

	private:
#ifdef _DEBUG
		static const char* HASHED_VS_NONHASHED[];
		inline void checkHash(const FastStr*) const;
		bool hashed;	/// produce an error if comparing hashed and non-hashed string
#endif
		int hash;		/// hash value of string, calculated on request

		void assign(const char*); /// assign c-string to FastStr (used by constructors)

		inline void getSubstr(FastStr&, int, const FastStr&); /// inline internal service function

		/* table exposing "whitespace" characters	*/;
		/* 0 = non-whitespace	*/;
		/* 1 = whitespace		*/;
		static const unsigned char WHITESPACE[];

		/* table for char -> num conversion */;
		static const unsigned char CHAR_TO_NUM[];

	public:
		int cursor;		/// self-advancing cursor within 'FastStr'
		char* str;		/// pointer to 1st character of string
		int size;		/// size of actual string
		
		// constructors
		/* - empty, to be initialized later */;
		FastStr();

		/* - find actual string size by itself	*/;
		/* => start = start of string			*/;
		/*	  calcHash = compute hash?			*/;
		FastStr(const char*, bool = true); /// uses 'assign' function

		/* - get size externally					*/;
		/* => start = start of string				*/;
		/*	  eSize = string size forced externally	*/;
		/*	  calcHash = compute hash?				*/;
		FastStr(const char*, unsigned int, bool = true);

		/* trim string by removing chars from start	and/or end of string */;
		/* => start	= number of chars to trim from start	*/;
		/*	  end	= number of chars to trim from end		*/;
		/* <= string size is updated accordingly			*/;
		void trim(int, int = 0);

		/* overloaded operators */;
		/// equation between two 'FastStr's
		bool operator==(const FastStr&) const;
		bool operator!=(const FastStr&) const;
		/// assignment
		void operator=(const FastStr&);
		void operator=(char*);
		/// compare
		bool operator<(const FastStr&) const;
		/// copy constructor
		FastStr(const FastStr&); /// issues the assignment operator

		/* convert to c-string				*/;
		/* <= new null terminated string	*/;
		/* Note : be careful to 'delete []' the created string when no longer needed, or there will be a memory leak */;
		char* toCString() const;

		/* copy string into a new buffer (should be of sufficient size!) */;
		/* => buffer = pointer to buffer	*/;
		/*	  nul = add null char at end?	*/;
		void copyString(char*, bool = true) const;

		/* get new copies of 'FastStr' and referenced string */;
		/* Note : be careful to 'delete []' the new referenced string when no longer needed, or there will be a memory leak */;
		FastStr* copy() const;

		static const FastStr STRING_CONTAINERS; /// default string containers (") and (')

		/* get a substring surrounded by "whitespace" - 'cursor' is updated				*/;
		/* => fstr = 'FastStr' reference												*/;
		/*	  start = start searching from this character onwards (maybe negative)		*/;
		/*	  containers = if 1st non-whitespace char is one of these chars, strings	*/;
		/*				   will end with same char regardless in-between chars			*/;
		void getSubstring(FastStr&, int = 0, const FastStr& = STRING_CONTAINERS);

		/* same as above, but starts from last 'cursor' position */;
		void getNextSubstring(FastStr&, const FastStr& = STRING_CONTAINERS);

		/* get position of a substring */;
		/* => fstr = 'FastStr' to match								*/;
		/*	  start = start searching from this character onwards	*/;
		/*	  end = search up to and including this char			*/;
		/* <= int position of substring, or -1 if not found			*/;
		int substringPosition(const FastStr&, const int& = 0, const int& = 0x7FFFFFFF) const;

		/* get position of a substring, in reverse search order : from end to start */;
		/* => fstr = 'FastStr' to match							*/;
		/*	  start = end searching at this character			*/;
		/*	  end = search from this character and backwards	*/;
		/* <= int position of substring, or -1 if not found		*/;
		int reverseSubstringPosition(const FastStr&, const int& = 0, const int& = 0x7FFFFFFF) const;

		/* get the first position of any of the characters of the provided string */;
		/* => fstr = 'FastStr' to match								*/;
		/*	  start = start searching from this character onwards	*/;
		/*	  end = search up to and including this char			*/;
		/* <= int position of substring, or -1 if not found			*/;
		int charPosition(const FastStr&, const int& = 0, const int& = 0x7FFFFFFF) const;

		/* get the last position of any of the characters of the provided string */;
		int reverseCharPosition(const FastStr&, const int& = 0, const int& = 0x7FFFFFFF) const;

		/* compute hash */;
		void calculateHash();

		/* count textline of a char					*/;
		/* => pos = position of char in 'FastStr'	*/;
		/*	  start = start counting from this char */;
		/* <= textline count of char (1st = 0)		*/;
		int calculateLine(int, int = 0) const;

		/// this function is <superseded> by the slower but much much much more powerful <Utils::fromNumber> function
		/* convert to number */;
		/* => t = class reference to accept the converted value */;
		/*	  radix = bin, dec, oct or hex						*/;
		/* <= was there a conversion error?						*/;
		enum RADIX {
			BINARY		= 2,
			OCTAL		= 8,
			DECIMAL		= 10,
			HEXADECIMAL	= 16,
		};
		template <class Class> bool toNumber(Class &t, FastStr::RADIX radix) {
			double sum = 0;
			double point = 0;
			double rad = 1;
	
			for (int i = size - 1; i >= 0 ; --i) {

				unsigned char num = CHAR_TO_NUM[ str[i] ]; /// char -> num

				if (num >= radix) {
					if (num == 98) continue;	/// ','
					if (num == 99) {			/// '.'
						if (point) { t = 0; return true; } /// more than 1 point
						point = rad;
						continue;
					}
					/// char exceeds radix (eg. 'A' in decimal), or generally illegal char (-1 / 255)
					t = 0;
					return true;
				}
				sum += num * rad; /// I really hope multiplication takes just 1 clock cycle on today's processors!!!
				rad *= radix;
			}

			if (point) sum /= point;
			t = (Class)sum; /// assign value
			return false;
		};

	};

	/* overloaded operators (global) */;
	/// cout FastStr
	std::ostream& operator<<(std::ostream&, const FastStr&);

}

#endif // FANLIB_FastStr_H