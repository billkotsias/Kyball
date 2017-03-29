#include "StdAfx.h"
#include "collision\Point.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include <math.h>

namespace P3D {

	Point::Point(Ogre::SceneNode *p) {
		point = p;
	}

	bool Point::isReady() {

		if (!point || !point->isInSceneGraph()) return false;
		return true;
	}

	Ogre::Vector3 Point::getWorldPosition() const {
		return point->_getDerivedPosition();
	}

	double Point::distance(const Ogre::Vector3 &p) const {
		return getWorldPosition().distance(p);
	}
}