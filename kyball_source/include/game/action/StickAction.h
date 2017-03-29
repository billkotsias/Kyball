//////////////
// StickAction
//////////////

#pragma once
#ifndef StickAction_H
#define StickAction_H

#include "Action.h"

namespace P3D {

	class StickAction : public Action {

	protected:

		Action::Level stickiness;

	public:

		// constructor
		StickAction(Action::Level level) : Action(Action::STICK) {
			stickiness = level;
		};

		// get data
		inline Action::Level getStickiness() { return stickiness; };

		virtual Action* copy() { return new StickAction(stickiness); };
	};
}

#endif