////////////////
// ReflectAction
////////////////

#pragma once
#ifndef ReflectAction_H
#define ReflectAction_H

#include "Action.h"

namespace P3D {

	class ReflectAction : public Action {

	protected:

		Action::Level reflection;

	public:

		// constructor
		ReflectAction(Action::Level level) : Action(Action::REFLECT) {
			reflection = level;
		};

		// get data
		inline Action::Level getReflection() { return reflection; };

		virtual Action* copy() { return new ReflectAction(reflection); };

	};
}

#endif