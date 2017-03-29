///////////
// Line
///////////

#pragma once
#ifndef Line_H
#define Line_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"
#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class Line : public CollisionObject {

	protected:

		Ogre::SceneNode* points[2];	/// defining start-end points

	public:

		// constructor
		// => p1, p2
		Line(Ogre::SceneNode*, Ogre::SceneNode*);

		virtual bool isReady();

		Ogre::Vector3 getWorldPoint(unsigned int) const; /// get a line's point in world co-ordinates

		/// 'I' am a 'Line'!
		#include "checkCollision.h"

		#include "distance.h"

		virtual double distance(const Ogre::Vector3&) const;
	};

}

#endif