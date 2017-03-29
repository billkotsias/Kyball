////////////////
// DestroyAction
////////////////

#pragma once
#ifndef DestroyAction_H
#define DestroyAction_H

#include "Action.h"

namespace P3D {

	class DestroyAction : public Action {

	protected:

		Action::Level power;

	public:

		// constructor
		DestroyAction(Action::Level level) : Action(Action::DESTROY) {
			power = level;
		};

		// get data
		inline Action::Level getPower() { return power; };

		virtual Action* copy() { return new DestroyAction(power); };

	};
}

#endif