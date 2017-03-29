////////
// Point
////////

#pragma once
#ifndef Point_H
#define Point_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"

namespace Ogre {
	class Vector3;
	class SceneNode;
}

namespace P3D {

	class Point : public CollisionObject {

	protected:

		Ogre::SceneNode* point;

	public:

		// constructor
		Point(Ogre::SceneNode*); /// This class is <NOT responsible> for the <deletion> of passed node

		virtual bool isReady();

		Ogre::Vector3 getWorldPosition() const;

		/// I am a 'Point'!
		#include "checkCollision.h"

		#include "distance.h"

		virtual double distance(const Ogre::Vector3&) const;
	};

}

#endif