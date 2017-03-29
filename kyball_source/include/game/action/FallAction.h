///////////
// FallAction
///////////

#pragma once
#ifndef FallAction_H
#define FallAction_H

#include "Action.h"

namespace P3D {

	class FallAction : public Action {

	public:

		double delay; /// delay till start of "fall"

		// constructor
		FallAction(double d) : Action(Action::FALL), delay(d) {};

	};
}

#endif