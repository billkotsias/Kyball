#include "StdAfx.h"
#include "collision\Cylinder.h"

namespace P3D {

	Cylinder::Cylinder(Ogre::SceneNode *p1, Ogre::SceneNode *p2, double inRad, bool neg) {
		points[0] = p1;
		points[1] = p2;
		radius = inRad;
		negativeRadius = neg;
	}

	bool Cylinder::isReady() {

		/// check if all required points are not null and in <sceneGraph>
		if (
			(points[0]) && (points[0]->isInSceneGraph()) &&
			(points[1]) && (points[1]->isInSceneGraph())
		   )
			return true;
		return false;
	}

	Ogre::Vector3 Cylinder::getWorldPoint(unsigned int pNum) const {
		return points[pNum]->_getDerivedPosition();
	}

	void Cylinder::virtualCacheParams() {
		/// worldRadius
		worldRadius = points[0]->_getDerivedScale().x * radius; /// NOTE : only <x scale> is used, because <uniform scale> is assumed
		if (negativeRadius) worldRadius = - worldRadius;
	}

	double Cylinder::distance(const Ogre::Vector3 &p) const {
		return CollisionExecuter::pointLineDistance(p, getWorldPoint(0), getWorldPoint(1)) - getWorldRadius();
	}

}