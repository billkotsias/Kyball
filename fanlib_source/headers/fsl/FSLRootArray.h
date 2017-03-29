// Fantasia Scripting Language - Root Array instance

#pragma once
#ifndef FANLIB_FSLRootArray_H
#define FANLIB_FSLRootArray_H

#include <fsl\FSLArray.h>

namespace FANLib {

	class FSLClassDefinition;

	class FSLRootArray : public FSLArray {
	protected:
		
		FSL::VAR_TYPE deepType; /// for PARSER usage
		int dimensions;			/// error-checking : number of dimensions
		FSLClassDefinition* definition; /// error-checking : used if 'deepType' = CLASS

	public:

		FSLRootArray();
		FSLRootArray(FSLRootArray*); /// copy main values

		friend class FSLParser;
	};
}

#endif