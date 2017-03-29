////////////
// FormState - message that affects a 'Form's appearance
////////////

#pragma once
#ifndef FormState_H
#define FormState_H

namespace P3D {

	class FormState {

	public:

		/* possible 'Action' types */;
		enum State {
			GENERIC,	/// => Form can't declare itself as a "special" object
			BALL,		/// => 'BallState'
		};

	protected:

		/* type of this action */;
		FormState::State state;

		// constructor
		FormState(FormState::State sta) {
			state = sta;
		};

	public:

		virtual ~FormState() {};

		inline FormState::State getState() { return state; };

		virtual FormState* copy() = 0; /// remember to <delete> this copy when no longer needed

	};
}

#endif