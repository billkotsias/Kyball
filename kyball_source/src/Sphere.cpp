#include "StdAfx.h"
#include "collision\Sphere.h"

namespace P3D {

	Sphere::Sphere(Ogre::SceneNode *inPos, double inRad, bool neg) {
		pos = inPos;
		radius = inRad;
		negativeRadius = neg;
	}

	bool Sphere::isReady() {

		/// check if all required points are not null and in <sceneGraph>
		if (pos && pos->isInSceneGraph()) return true;

		return false;
	}

	Ogre::Vector3 Sphere::getWorldPosition() const {
		return pos->_getDerivedPosition();
	}

	void Sphere::virtualCacheParams() {
		/// worldRadius
		worldRadius = pos->_getDerivedScale().x * radius; /// NOTE : only <x scale> is used, because <uniform scale> is assumed
		if (negativeRadius) worldRadius = - worldRadius;
	}

	double Sphere::distance(const Ogre::Vector3 &p) const {
		return getWorldPosition().distance(p) - getWorldRadius();
	}

}