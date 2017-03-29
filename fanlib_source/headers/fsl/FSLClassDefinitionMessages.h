// FSLClassDefinition Messages

#pragma once
#ifndef FANLIB_FSLCLASSDEFINITIONMESSAGES_H
#define FANLIB_FSLCLASSDEFINITIONMESSAGES_H

namespace FANLib {

	const char* const FSLClassDefinition::VAR_NAME_EXISTS[] = {"Variable name '","' already exists", 0};
	const char* const FSLClassDefinition::VAR_NOT_FOUND[] = {"Variable name '","' not found", 0};
	const char* const FSLClassDefinition::VAR_IS_DEF[] = {"Variable name '","' is the default value of a Class Definition", 0};
}

#endif