#include "StdAfx.h"
#include "game\ai\AIPos.h"

#include "game\form\Form.h"
#include "collision\CollisionObject.h"
#include "OgreAddOns.h"
#include "OgreBuilder.h"

#include <OgreSceneNode.h>

namespace P3D {

	AIPos::AIPos(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position) : AI(obj) {
		pos = parent->createChildSceneNode(position);
	}

	AIPos::~AIPos() {
		attachForm(0); /// detach any attached form; <NOTE> : This must happen BEFORE destroying 'pos'

		OgreBuilder::destroySceneNode(pos); //was: pos->getParentSceneNode()->removeAndDestroyChild(pos->getName());
	}

	void AIPos::setPosition(const Ogre::Vector3& position) {
		pos->setPosition(position);
	}

	const Ogre::Vector3& AIPos::getPosition() {
		return pos->getPosition();
	}

	void AIPos::setOrientation(const Ogre::Quaternion& orient) {
		pos->setOrientation(orient);
	}

	const Ogre::Quaternion& AIPos::getOrientation() {
		return pos->getOrientation();
	}

	Ogre::Vector3 AIPos::getWorldPosition() {
		return OgreAddOns::getDerivedPosition(pos);
	}

	void AIPos::setWorldPosition(const Ogre::Vector3& position) {
		OgreAddOns::setDerivedPosition(pos, &position);
	}

	bool AIPos::attachForm(Form* inForm) {

		if (AI::attachForm(inForm)) return true; /// remove previously attached 'Form'

		form = inForm;
		if (!form) return true; /// if <Null> <= quit recursive calling

		/// attach depiction to <pos> 'SceneNode'
		/// - attach <pivot>
		Ogre::SceneNode* formPivot = form->getPivot();
		if (formPivot->getParent()) formPivot->getParent()->removeChild(formPivot);
		pos->addChild(formPivot);
		/// - set "checkFutureCollision" to Form to match mine (we have a position so its logic to put this here)
		form->getCollisionObject()->checkFutureCollision = checkForFutureCollision();

		return false;
	}
}