#include "StdAfx.h"
#include "game\cannon\BasicBallCreator.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\ai\BallAI.h"
#include "game\GameplayCollection.h"
#include "game\form\BallForm.h"
#include "tween\TLinear.h"
#include <OgreBillboard.h>
#include "game\cannon\Cannon.h"

namespace P3D {

	const int BasicBallCreator::CREATED_RECORD_SIZE = 8;

	void BasicBallCreator::calcFutureBallType() {

		/// non-random balls
		if (!incomingBalls.empty()) {
			futureType = incomingBalls.front();
			incomingBalls.pop_front();
			return;
		}


		/// out of non-random balls : select randomly from available "stuck" balls in level
		std::map<BallState::BallType, int> existingTypesMap;

		/// - consider only "stuck" BallAIs
		const std::map<GameplayObject*, bool>& balls = gameplayBase->getBalls();
		for (std::map<GameplayObject*, bool>::const_iterator it = balls.begin(); it != balls.end(); ++it) {

			GameplayObject* obj = it->first;
			BallAI* ballAI = (BallAI*)obj->getAI();
			if (ballAI->getInternalState() == BallAI::STUCK) {

				/// is this a normal colour ball?
				BallState::BallType ballType = ((BallState*)obj->getFormState())->getBallType();
				if (ballType < BallState::BOB)
					++existingTypesMap[ ballType ]; /// register colored ball <!>
			}
		}

		if (existingTypesMap.empty()) return; /// keep the same type

		/// - copy map to vector but split <probability> inversely proportional to record
		std::deque<BallState::BallType> existingTypes;
		{
			for (std::map<BallState::BallType, int>::iterator it = existingTypesMap.begin(); it != existingTypesMap.end(); ++it) {

				BallState::BallType ballType = it->first;
				int typeProbability = createdRecord.size() - getTypeFreq(ballType); /// invert frequency to infrequency
				typeProbability *= typeProbability;

				for (int i = typeProbability; i >= 0; --i) {
					existingTypes.push_back(ballType);
				}
			}
		}

		int typeIndex = floor( ( (double)(rand() * existingTypes.size()) ) / RAND_MAX ); /// <equal> probability for all balltypes
		futureType = existingTypes.at(typeIndex);

		createdRecord.push_back(futureType);
		if (createdRecord.size() > CREATED_RECORD_SIZE) createdRecord.pop_front();
	}

	unsigned int BasicBallCreator::getTypeFreq(BallState::BallType type) {

		unsigned int freq = 0;

		for (int i = createdRecord.size() - 1; i >= 0; --i) {
			if (createdRecord.at(i) == type) ++freq;
		}

		return freq;
	}
}