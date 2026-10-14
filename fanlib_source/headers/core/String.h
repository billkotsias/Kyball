#pragma once
#ifndef FANLIB_String_H
#define FANLIB_String_H

#include <string>

namespace FANLib {

	class String {

	public:

		template <class Class>
		static Class& replaceAll(Class& string, const Class& toReplace, const Class& withThis)
		{
			typename Class::size_type n = 0;
			while ( ( n = string.find( toReplace, n ) ) != Class::npos )
			{
				string.replace( n, toReplace.size(), withThis );
				n += withThis.size();
			}

			return string;
		};

		template <class Class>
		static Class& setCase(Class& string, bool upperCase)
		{
			int (*caseFunc)(int);
			caseFunc = (upperCase) ? ::toupper : ::tolower;

			typename Class::iterator i = string.begin();
			const typename Class::iterator end = string.end();

			while (i != end) {
				*i = caseFunc((unsigned char)*i);
				++i;
			}

			return string;
		};

		//static std::string& replaceAll(std::string& string, const std::string& toReplace, const std::string& withThis);
		//static std::string& setCase(std::string& string, bool upperCase);
	};
}

#endif