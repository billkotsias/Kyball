#include "StdAfx.h"
#include "collision\CollisionParams.h"

#include "game\GameplayObject.h"

namespace P3D {

	const CollisionParams CollisionParams::NO_COLLISION = CollisionParams();

	void CollisionParams::calculateLambda() {
		/// 'Infinite' masses allowed
		Ogre::Vector3 u1 = gameObj1->getWorldVelocity();
		Ogre::Vector3 u2 = gameObj2->getWorldVelocity();
		lambda = 2 * (u1 - u2).dotProduct(normal) / ( (1./gameObj1->getMass() + 1./gameObj2->getMass()) * normal.dotProduct(normal) );
	}
}