#include "StdAfx.h"
#include "game\ai\BroAI.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\action\ReflectAction.h"
#include "game\action\NoAction.h"
#include "visual\VisualBoss.h"
#include "visual\BallBoom.h"
#include "BurstBoss.h"

#include "sound\Sound.h"

namespace P3D {

	BroAI::BroAI(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position) : BallAI(obj, BallAI::STUCK, parent, position, Infinite) {
	}

	void BroAI::changeInternalState(BallAI::InternalState inState)
	{
		switch (inState)
		{
			case STUCK:
				changeAction(new ReflectAction(Action::LOW)); /// this is the <only> difference with BallAI
				checkCollision = AI::ACCEPT;
				if (form) link();

				internalState = inState;
				break;

			default:
				BallAI::changeInternalState(inState);
				break;
		}
	}

	void BroAI::onDestroy(Action* action)
	{
		/// create an according <explosion> animation in my place as visual-feedback for my destruction
		GameplayBase* base = owner->getGameplayBase();

		/// - <boom>
		VisualBoss::getSingletonPtr()->insert( new BallBoom(base, BallState::WHITE, getPosition(), 2) );

		/// - <burst>
		base->getBurstBoss()->newBurst(BurstBoss::BOB, pos->getParentSceneNode(), pos->getPosition());
		Sound::BALL_BIG_POP->play( getWorldPosition() ); /// sound

		prepareToDie();
	}

}
