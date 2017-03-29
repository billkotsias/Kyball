// Fantasia Scripting Language - FSLInstance
// - holds all essential data to access or alter an FSL structure

#pragma once
#ifndef FANLIB_FSLInstance_H
#define FANLIB_FSLInstance_H

#include <map>
#include <core\FastStr.h>

namespace FANLib {

	class FSLEnum;
	class FSLClassDefinition;

	class FSLInstance {

	private:

		static const char* MISSING_ENUM[];
		static const char* MISSING_DEF[];

		// => TO BE MOVED TO 'FSLInstance'
		/// <Class definitions> holder
		std::map<FastStr, FSLClassDefinition*> classDefinitions; /// class definitions have a pointer to themselves, so can't be accidentally GCed

		/// <enumerations> holder
		std::map<FastStr, FSLEnum*> enums;

	public:

		FSLInstance::~FSLInstance();

		// get a class definition
		FSLClassDefinition* getClassDef(const FastStr&) const;
		FSLClassDefinition* getClassDef(const char*) const;

		// get root class (definition)
		FSLClassDefinition* getRoot() const { return getClassDef(FastStr("")); };

		// get an enumeration
		// => fstr = enumeration name
		// <= enumeration instance
		FSLEnum* getEnum(const FastStr&) const;
		FSLEnum* getEnum(const char*) const;

		friend class FSLParser;
	};

}

#endif