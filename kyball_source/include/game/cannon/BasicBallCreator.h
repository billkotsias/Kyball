// ------
// BasicBallCreator
// ------

#pragma once
#ifndef BasicBallCreator_H
#define BasicBallCreator_H

#include "BallCreator.h"

#include <OgreVector2.h>

namespace Ogre{
	class Billboard;
}

namespace P3D {

	class BasicBallCreator : public BallCreator {

	protected:

		std::deque<BallState::BallType> createdRecord; /// keep record of last 8 created Balls, in order to balance randomness!

		unsigned int getTypeFreq(BallState::BallType); /// get frequency of given balltype in "created record"

	public:

		static const int CREATED_RECORD_SIZE;

		// constructor
		// => pointer to a 'GameplayBase'
		BasicBallCreator(GameplayBase* base) : BallCreator(base) {};

		// overrides
		virtual void calcFutureBallType();

	};

}

#endif