// ------
// BallCreator
// ------

#pragma once
#ifndef BallCreator_H
#define BallCreator_H

#include <OgreVector3.h>
#include <deque>
#include "game\form\BallState.h"

namespace FANLib {
	class FSLArray;
}

namespace P3D {

	class GameplayBase;
	class GameplayObject;
	class BallAI;
	class BallForm;

	class BallCreator {

	protected:

		GameplayBase* gameplayBase;
		std::deque<GameplayObject*> gameplayObjects; /// 'GameplayObject's created for the 'Cannon' !
		/// <TODO> : add a 'Billboard' 'Animator' to this class!

		BallState::BallType futureType;

		std::deque<BallState::BallType> incomingBalls;

	public:

		// constructor
		// => pointer to a 'GameplayBase'
		BallCreator(GameplayBase*);
		virtual ~BallCreator();

		// set array of non-random incoming balls
		virtual void setIncomingBalls(FANLib::FSLArray*);

		// calculate future ball type
		virtual void calcFutureBallType() = 0;

		// get future ball type
		inline BallState::BallType getFutureBallType() { return futureType; };

		// create a 'BallAI' for 'Cannon'
		static GameplayObject* create(GameplayBase*, BallState::BallType); /// generic function
		virtual GameplayObject* create();

		// destroy all 'GameplayObject's created by this class
		void destroyAll();
	};

}

#endif