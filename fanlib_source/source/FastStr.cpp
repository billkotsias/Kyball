#include "core/FastStr.h"
#include "core/Utils.h"

#ifdef _DEBUG
	// error-handling
	#include <core\Log.h>
	#include <FANLibErrors.h>
	#include <core\FastStrMessages.h>
#endif

namespace FANLib {

	// table exposing "whitespace" characters
	// 0 = not whitespace
	// 1 = whitespace
	const unsigned char FastStr::WHITESPACE[255] = {
		/*  0  1  2  3  4  5  6  7 */ 
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1,
	/*128*/ 0, 0, 0, 0, 0, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
	/*160*/	1						/*the rest are considered non-whitespace*/,
	};

	// char to num table
	const unsigned char FastStr::CHAR_TO_NUM[255] = {
		/*  0  1  2  3  4  5  6  7 */ 
		   -1,-1,-1,-1,-1,-1,-1,-1, /// -1 -> 255 (unsigned char)
		   -1, 0, 0, 0, 0, 0,-1,-1, /// excuse whitespace as 0 (don't overdo it!)
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		    0,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,98,-1,99,-1, /*',' '.'*/ 
	/*48*/	0, 1, 2, 3, 4, 5, 6, 7,
			8, 9,-1,-1,-1,-1,-1,-1,
	/*64*/ -1,10,11,12,13,14,15,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,10,11,12,13,14,15,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1, /// that's enough for error-checking!
	};
	// constructors
	FastStr::FastStr() : str(0), size(0), cursor(0), hash(0) {
#ifdef _DEBUG
		hashed = false;
#endif
	}
	
	FastStr::FastStr(const char* start, bool calcHash) : cursor(0), hash(0) {
#ifdef _DEBUG
		hashed = false;
#endif
		assign(start);
		if (calcHash) calculateHash(); /// calc hash?
	}

	FastStr::FastStr(const char* start, unsigned int eSize, bool calcHash) : cursor(0), hash(0), size(eSize) {
#ifdef _DEBUG
		hashed = false;
#endif
		str = (char*)start;
		if (calcHash) calculateHash(); /// calc hash?
	}

	FastStr::FastStr(const FastStr& fstr) {
		*this = fstr;
	}

	void FastStr::assign(const char* start) {
		str = (char*)start;
		size = 0;						/// determine size of string
		while (str[size] != 0) size++;	/// - ends with null-char
	}

	// trim string
	void FastStr::trim(int start, int end) {
		if (start > size) start = size;
		str += start;
		size -= start + end;
		if (size < 0) size = 0;
	}

	// copy string into a new buffer of sufficient size
	void FastStr::copyString(char* buffer, bool nul) const {
		for (int i = 0; i < size; ++i) {
			buffer[i] = str[i];
		}
		if (nul) { buffer[size] = 0; }
	}

	// convert to c-string
	char* FastStr::toCString() const {
		char* cstr = new char [size + 1];
		copyString(cstr, true);
		return cstr;
	}

	// create exact new copies of FastStr and referenced string
	FastStr* FastStr::copy() const {
		FastStr* fstr = new FastStr(this->toCString(), this->size, false);
		fstr->hash = this->hash;
		fstr->cursor = this->cursor;
#ifdef _DEBUG
		fstr->hashed = this->hashed;
#endif
		return fstr;
	}

	// overloaded operators
	/* == */;
	bool FastStr::operator==(const FANLib::FastStr &fstr) const {

//#ifdef _DEBUG
//		static const char* COMPARING[] = {"Comparing '","' <> '","'",0};
//		const FastStr* comparing[] = {this, &fstr,0};
//		Log::internalLog(COMPARING, comparing, true, 0);
//#endif

#ifdef _DEBUG
		checkHash(&fstr);
#endif
		if (hash != fstr.hash) return false;
		if (size != fstr.size) return false; /// mainly in case hash hasn't been calculated in any of the 2 objects
		for (int p = size - 1;  p >= 0; --p) {
			if (str[p] != fstr.str[p]) return false;
		}
		return true;
	}

	/* != */;
	bool FastStr::operator !=(const FANLib::FastStr &fstr) const {
		return !(*this == fstr);
	}

	/* = */;
	void FastStr::operator=(const FANLib::FastStr &fstr) {
		str = fstr.str;
		size = fstr.size;
		hash = fstr.hash;
		cursor = fstr.cursor;
#ifdef _DEBUG
		hashed = fstr.hashed;
#endif
	}
	void FastStr::operator=(char* start) {
		assign(start);
		hash = 0;
		cursor = 0;
	}

	/* < */;
	bool FastStr::operator<(const FANLib::FastStr& fstr) const {
#ifdef _DEBUG
		checkHash(&fstr);
#endif
		if (hash != fstr.hash) return (hash < fstr.hash);
		if (size != fstr.size) return (size < fstr.size);
		for (int p = size - 1;  p >= 0; --p) {
			if (str[p] != fstr.str[p]) return (str[p] < fstr.str[p]);
		}
		return false; /// the 2 objects contain equal strings
	}

	/* cout << (global function!) */;
	std::ostream& operator<<(std::ostream& os, const FastStr& fstr) {
		for (int i = 0; i < fstr.size; ++i) {
			os << fstr.str[i];
		}
		return os;
	}

#ifdef _DEBUG
	inline void FastStr::checkHash(const FastStr* fstr) const {
		if (hashed ^ fstr->hashed) {
			const FastStr* hashed_vs_nonhashed[] = {fstr, this, 0};
			if (hashed) {
				hashed_vs_nonhashed[0] = this;
				hashed_vs_nonhashed[1] = fstr;
			}
			Log::internalLog(HASHED_VS_NONHASHED, hashed_vs_nonhashed, true, FANLib::Error::FASTSTR_HASHED_VS_NONHASHED);
		}
	}
#endif

	const FastStr FastStr::STRING_CONTAINERS("\"'");

	// inline function for getting a substring
	// => fstr = target 'FastStr' reference
	//	  start = start searching from this character onwards (maybe negative)
	//	  containers = if 1st non-whitespace char is one of these chars, strings will end with same char regardless in-between chars
	//	  NOTE : containers are NOT returned in the 'FastStr'
	inline void FastStr::getSubstr(FastStr& fstr, int start, const FastStr& containers) {

		/// remove preceding whitespace
		while ( start < size && WHITESPACE[ (unsigned char)str[start] ] ) {++start;};
		if (start >= size) {
			fstr.size = 0; /// return empty substring, 'cursor' points to end of string
			fstr.hash = 0;
			cursor = size;
			return;
		}
		fstr.str = str + start; /// set start of substring
		cursor = start + 1; /// 'cursor' is updated

		/// check if substring is bounded in a container
		FastStr firstChar(str + start, (unsigned int)1);
		if ( containers.substringPosition(firstChar) >= 0) {
			/// substring is container bound
			while ( cursor < size && str[cursor] != fstr.str[0] ) {++cursor;};
			++fstr.str;
			++start;
			fstr.size = cursor - start; /// store string size
			if (cursor < size) ++cursor;
		} else {
			/// substring is whitespace bound
			while ( cursor < size && !WHITESPACE[ (unsigned char)str[cursor] ] ) {++cursor;};
			fstr.size = cursor - start; /// store string size
		}

		fstr.hash = 0;
#ifdef _DEBUG
		fstr.hashed = false;
#endif
	}

	// get a meaningful substring
	// => fstr = target 'FastStr' reference
	//	  start = start searching from this character onwards (maybe negative)
	//	  containers = if 1st non-whitespace char is one of these chars, strings will end with same char regardless in-between chars
	void FastStr::getSubstring(FastStr& fstr, int start, const FastStr& containers) {
		getSubstr(fstr, start, containers);
	}

	// get next meaningful substring
	// => fstr = target 'FastStr' reference
	//	  start = start searching from this character onwards (maybe negative)
	//	  containers = if 1st non-whitespace char is one of these chars, strings will end with same char regardless in-between chars
	void FastStr::getNextSubstring(FastStr& fstr, const FastStr& containers) {
		getSubstr(fstr, cursor, containers);
	}

	// get position of a substring
	// => fstr = 'FastStr' to match
	//	  start = start searching from this character onwards
	//	  end = search up to and including this char
	// <= int position of substring, or -1 if not found
	int FastStr::substringPosition(const FastStr& fstr, const int& start, const int& end) const {
		int endChar = (size <= end) ? size : end;
		for (int pos = start; pos <= endChar - fstr.size; ++pos) {
			int i;
			for (i = fstr.size - 1; i >= 0; --i) {
				if ( str[ pos + i ] != fstr.str[i]) break;
			}
			if (i < 0) return pos; /// exact match found
		}
		return -1;
	}

	int FastStr::reverseSubstringPosition(const FANLib::FastStr &fstr, const int &start, const int &end) const {
		int endChar = (size <= end) ? size : end;
		for (int pos = endChar - fstr.size; pos >= start; --pos) { /// this is the only line that changes from the function above
			int i;
			for (i = fstr.size - 1; i >= 0; --i) {
				if ( str[ pos + i ] != fstr.str[i]) break;
			}
			if (i < 0) return pos; /// exact match found
		}
		return -1;
	}

	// get the first position of any of the characters of the provided string
	// => fstr = 'FastStr' containg the characters of interest
	//	  start = start searching from this character onwards
	//	  end = search up to and including this char
	// <= int position of 1st character met, or -1 if none found
	int FastStr::charPosition(const FastStr& fstr, const int& start, const int& end) const {
		int endChar = (size <= end) ? size : end;
		for (int pos = start; pos <= endChar - fstr.size; ++pos) {
			for (int i = 0; i < fstr.size; ++i) {
				if ( str[pos] == fstr.str[i]) return pos;
			}
		}
		return -1;
	}

	// get the last position of any of the characters of the provided string
	int FastStr::reverseCharPosition(const FastStr& fstr, const int& start, const int& end) const {
		int endChar = (size <= end) ? size : end;
		for (int pos = endChar - fstr.size; pos >= start; --pos) {
			for (int i = fstr.size - 1; i >= 0 ; --i) {
				if ( str[pos] == fstr.str[i]) return pos;
			}
		}
		return -1;
	}

	// calculate hash
	void FastStr::calculateHash() {
#ifdef _DEBUG
		hashed = true;
#endif
		hash = Utils::hash(str, size);
	}

	// calculate textline of a char
	// note :  chars 10, 13 and 13+10 are counted as textlines
	int FastStr::calculateLine(int pos, int start) const {
		int count = 0;
		if (pos > size) pos = size;
		if (start < 0) start = 0;
		while (start < pos) {
			char ch = str[start];
			if (ch == 10) {
				++count;
			} else if (ch == 13) {
				++count;
				if ((start < (pos - 1)) && str[start + 1] == 10) ++start; /// jump '10' if it is after a '13'
			}
			++start;
		}
		return count;
	}

}