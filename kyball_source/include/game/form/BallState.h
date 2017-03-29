////////////
// BallState - normal(?) gameplay 'Ball'
////////////

#pragma once
#ifndef BallState_H
#define BallState_H

#include "FormState.h"
#include <vector>
#include <OgreColourValue.h>

namespace P3D {

	class BallState : public FormState {

	public:

		/* all 'Ball' types */;
		enum BallType {

			/// normal balls
			RED = 0,
			GREEN,
			BLUE,
			CYAN,
			MAGENTA,
			YELLOW,
			WHITE,
			BLACK,

			BOB,		/// directly indestructible ball - but STICKY
			BRO,		/// directly indestructible ball - but BOUNCY

			UNCHANGED,	/// don't change the colour of this Ball

			/// NOTE : not sure if this state should include "special" balls, maybe they are different state & form
			_last,
		};

	protected:

		BallState::BallType ballType;	/// ball type (colour)
		double radius;					/// ball radius
		bool countForScore;				/// will this ball be counted in score calculation? (is it a level-ball or a cannon-ball!?)

	public:

		static const double BALL_RADIUS;	/// default ball radius in Ogre units
		static const int MAX_TYPES = 8;		/// number of different types of balls (should be 8)
		static const int MAX_BOBS = 2;		/// additional types of "balls"

		/// integer [0...7] to <BallType> conversion table
		static const BallState::BallType ballTypes[BallState::MAX_TYPES + BallState::MAX_BOBS];

		/// function for creating <BallType> => <Ogre::ColourValue> tables
		static const std::vector<Ogre::ColourValue> initBallColours(double, double, double, double);

		// constructor
		BallState::BallState(BallType type, bool count = false, double inRad = BALL_RADIUS) : FormState(FormState::BALL), ballType(type), radius(inRad), countForScore(count) {
		};

		inline void setBallType(BallState::BallType inType)	{ ballType = inType; };
		inline BallState::BallType getBallType()			{ return ballType; };
		inline double getBallRadius()						{ return radius; };
		inline bool getCountForScore()						{ return countForScore; };

		virtual FormState* copy() { return new BallState(ballType, countForScore, radius); }; /// remember to <delete> this copy when no longer needed

	};
}

#endif