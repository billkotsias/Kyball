#include "StdAfx.h"
#include "game\ai\AIPosVel.h"

#include "game\GameplayObject.h"
#include "game\GameplayBase.h"
#include "game\ai\AIPVAnimator.h" /// needed for AIPVAnime clean-up

/// needed for "link"/"unlink" requests!
#include "game\event\EventManager.h"
#include "game\event\LinkEvent.h"
#include "game\event\UnlinkEvent.h"
#include "game\event\MoverEvent.h"

#include "game\form\Form.h"
#include "collision\CollisionObject.h"
#include "OgreBuilder.h"
#include "P3DApp.h"

#include <OgreSceneNode.h>

namespace P3D {

	AIPosVel::AIPosVel(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position, bool mover) : AIPos(obj, parent, position) {
		vel = parent->createChildSceneNode(); /// we want the AI to rotate/scale itself without altering its velocity

		if (mover) owner->getGameplayBase()->getEventManager()->incomingEvent(new MoverEvent(this->owner));
	}

	AIPosVel::~AIPosVel() {
		attachForm(0); /// detach any attached form; <NOTE> : This must happen BEFORE destroying 'pos' & 'vel' SceneNodes
		owner->getGameplayBase()->getAIPVAnimator()->deleteAllAnimes(this);

		OgreBuilder::destroySceneNode(vel);
	}

	void AIPosVel::setVelocity(const Ogre::Vector3& velocity) {
		vel->setPosition(velocity);
	}

	const Ogre::Vector3& AIPosVel::getVelocity() {
		return vel->getPosition();
	}

	void AIPosVel::run(double percent) {
		pos->translate(vel->getPosition() * percent); /// add velocity portion to position
		//pos->translate(vel->getPosition() * percent + 0.5 * acc->getPosition() * percent * percent); /// add velocity portion to position
		//vel->translate(acc->getPosition() * percent);
	}

	void AIPosVel::link() {
		if (linked) return;
		owner->getGameplayBase()->getEventManager()->incomingEvent(new LinkEvent(this->owner));
		linked = true;
	}

	void AIPosVel::unlink() {
		if (!linked) return;
		owner->getGameplayBase()->getEventManager()->incomingEvent(new UnlinkEvent(this->owner));
		linked = false;
	}

	bool AIPosVel::attachForm(Form* inForm) {

		if (AIPos::attachForm(inForm)) return true; /// remove previously attached 'Form' & attach to <pos> SceneNode

		/// - attach <velocity>
		form->getCollisionObject()->setVelocity(vel);

		return false;
	}

}