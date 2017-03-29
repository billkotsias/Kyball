#include "StdAfx.h"
#include "game\ai\BobAI.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "visual\VisualBoss.h"
#include "visual\BallBoom.h"
#include "BurstBoss.h"

#include "sound\Sound.h"

namespace P3D {

	BobAI::BobAI(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position, double inMass) : BallAI(obj, BallAI::STUCK, parent, position, inMass) {
	}

	void BobAI::onDestroy(Action* action) {

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
