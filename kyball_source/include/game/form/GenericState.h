///////////////
// GenericState - nothing special
///////////////

#pragma once
#ifndef GenericState_H
#define GenericState_H

#include "FormState.h"

namespace P3D {

	class GenericState : public FormState {

	public:

		GenericState() : FormState(GENERIC) {};
		virtual FormState* copy() { return new GenericState(); };

	};
}

#endif