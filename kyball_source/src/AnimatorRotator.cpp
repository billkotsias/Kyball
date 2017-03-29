#include "StdAfx.h"
#include "visual\AnimatorRotator.h"
#include "visual\VisualBoss.h"
#include "OgreAddOns.h"

namespace P3D {

	AnimatorRotator::AnimatorRotator(Ogre::SceneNode* _node, const Ogre::Vector3& _axis, double speed, double start) : node(_node), axis(_axis), sp(speed) {

		if (start == start) {
			run( start * Ogre::Math::PI / (180. * sp) );
		} else {
			run( rand() * Ogre::Math::TWO_PI / (double)(RAND_MAX * sp) );
		}

		VisualBoss::getSingletonPtr()->insert(this);
	}

	bool AnimatorRotator::run(double time) {

		node->rotate(axis, -Ogre::Radian(sp * time));

		return false;
	}
}