#include "StdAfx.h"
#include "collision\Polygon.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>

namespace P3D {

	Polygon::Polygon(int pNum, Ogre::SceneNode** p) {
		if (pNum < 3) throw "Polygon must have at least 3 points"; /// catch crash early
		pointsNum = pNum;
		points = p;
		cachedPoints = new Ogre::Vector3[pNum];
	}

	Polygon::~Polygon() {
		delete [] cachedPoints;
	}

	bool Polygon::isReady() {

		/// check if all required points are not null and in <sceneGraph>
		for (int i = pointsNum - 1; i >= 0; --i) {
			if (!(&points[i]) || !(points[i]->isInSceneGraph())) return false;
		}
		return true;
	}

	void Polygon::virtualCacheParams() {
		/// cache points
		for (int i = pointsNum - 1; i >= 0; --i) {
			cachedPoints[i] = points[i]->_getDerivedPosition();
			cachedPoints[i] = cachedPoints[i];
		}
		/// cache normal
		worldNormal = (cachedPoints[1] - cachedPoints[0]).crossProduct(cachedPoints[2] - cachedPoints[1]);
		worldNormal.normalise();
	}

	double Polygon::distance(const Ogre::Vector3 &p) const {
		return CollisionExecuter::pointPolygonDistance(p, pointsNum, cachedPoints, getWorldNormal());
	}
}