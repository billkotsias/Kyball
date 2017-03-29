#include "StdAfx.h"
#include "game\cannon\BallCreator.h"

#include "game\ai\BallAI.h"
#include "game\form\BallForm.h"
#include "game\GameplayObject.h"
#include "game\GameplayBase.h"

#include "fsl\FSLArray.h"

namespace P3D {

	BallCreator::BallCreator(GameplayBase* base) : gameplayBase(base), futureType(BallState::RED) {
		gameplayBase->insertGameplayObjects(&gameplayObjects);
	}

	BallCreator::~BallCreator() {
		destroyAll();
		gameplayBase->eraseGameplayObjects(&gameplayObjects);
	}

	void BallCreator::destroyAll() {
		/// created
		for (int i = gameplayObjects.size() - 1; i >= 0 ; --i) {
			delete gameplayObjects.at(i);
		}
		gameplayObjects.clear();

		/// yet-to-be created
		incomingBalls.clear();
	}

	void BallCreator::setIncomingBalls(FANLib::FSLArray* fslIncoming) {
		for (unsigned int i = 0; i < fslIncoming->getSize(); ++i) {
			incomingBalls.push_back( (BallState::BallType)(fslIncoming->getInt(i)) );
		}
	}

	GameplayObject* BallCreator::create(GameplayBase* gameplayBase, BallState::BallType ballType) {

		/// create 'GameplayObject' (ai and form "container")
		GameplayObject* obj = new GameplayObject(gameplayBase);

		BallAI* ballAI = new BallAI(obj, BallAI::OUT_OF_PLAY, gameplayBase->getRootNode(), Ogre::Vector3::ZERO);
		obj->setAI(ballAI);

		BallForm* ballForm = new BallForm(gameplayBase, gameplayBase->getBBSet(GameplayBase::BALLS_ALPHA), ballType);
		obj->setForm(ballForm);

		return obj;
	}

	GameplayObject* BallCreator::create() {

		/// create 'GameplayObject'
		GameplayObject* obj = BallCreator::create(gameplayBase, futureType);
		gameplayObjects.push_back(obj);

		return obj;
	}
}