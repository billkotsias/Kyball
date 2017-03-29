#include "StdAfx.h"
#include "collision\CollisionObject.h"

namespace P3D {

	CollisionObject::CollisionObject() : velocity(0), checkFutureCollision(false) {
	}

	void CollisionObject::setVelocity(Ogre::SceneNode *vel) {
		velocity = vel;
	}

	Ogre::SceneNode* CollisionObject::getVelocity() {
		return velocity;
	}

	void CollisionObject::cacheVelocity() {
		if (!velocity || !velocity->getParentSceneNode()) {
			worldVelocity = Ogre::Vector3::ZERO; /// I am not supposed to know this, but we check for NULL because the <AI> 'we' are attached to may have no velocity at all!
		} else {
			worldVelocity = velocity->_getDerivedPosition() - velocity->getParentSceneNode()->_getDerivedPosition();
		}
	}

	void CollisionObject::cacheParams() {
		cacheVelocity();
		virtualCacheParams();
	}
}