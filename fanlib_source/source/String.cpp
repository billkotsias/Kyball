#include "core\String.h"

namespace FANLib {

	//std::string& String::replaceAll(std::string& string, const std::string& toReplace, const std::string& withThis)
	//{
	//	std::string::size_type n = 0;
	//	while ( ( n = string.find( toReplace, n ) ) != std::string::npos )
	//	{
	//		string.replace( n, toReplace.size(), withThis );
	//		n += withThis.size();
	//	}

	//	return string;
	//}

	//std::string& String::setCase(std::string& string, bool upperCase)
	//{
	//	int (*caseFunc)(int);
	//	caseFunc = (upperCase) ? ::toupper : ::tolower;

	//	std::string::iterator i = string.begin();
	//	const std::string::iterator end = string.end();

	//	while (i != end) {
	//		*i = caseFunc((unsigned char)*i);
	//		++i;
	//	}

	//	return string;
	//}

}