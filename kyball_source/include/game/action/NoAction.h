///////////
// NoAction
///////////

#pragma once
#ifndef NoAction_H
#define NoAction_H

#include "Action.h"

namespace P3D {

	class NoAction : public Action {

	public:

		// constructor
		NoAction() : Action(Action::NO_ACTION) {};

	};
}

#endif