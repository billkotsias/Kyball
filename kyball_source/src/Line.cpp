#include "StdAfx.h"
#include "collision\Line.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include <math.h>

namespace P3D {

	Line::Line(Ogre::SceneNode *p0, Ogre::SceneNode *p1) {
		points[0] = p0;
		points[1] = p1;
	}

	bool Line::isReady() {

		if (!points[0] || !points[0]->isInSceneGraph() ||
			!points[1] || !points[1]->isInSceneGraph()) return false;
		return true;
	}

	Ogre::Vector3 Line::getWorldPoint(unsigned int p) const {
		return points[p]->_getDerivedPosition();
	}

	double Line::distance(const Ogre::Vector3& p) const {
		return CollisionExecuter::pointLineDistance(p, getWorldPoint(0), getWorldPoint(1));
	}
}