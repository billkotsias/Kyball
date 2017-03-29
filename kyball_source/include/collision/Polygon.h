////////////////
// Polygon - the second most important 'CollisionObject' subclass
////////////////

#pragma once
#ifndef Polygon_H
#define Polygon_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"

namespace Ogre {
	class Vector3;
	class SceneNode;
}

namespace P3D {

	class Polygon : public CollisionObject {

	protected:

		Ogre::Vector3 worldNormal;	/// cached

	public:

		int pointsNum;				/// number of points the polygon is consisted of
		Ogre::SceneNode** points;	/// polygon's actual consisting points; must be defined <clockwise>

		Ogre::Vector3* cachedPoints; /// they are only cached in order to be kept together ('CollisionExecuter' requirement)

		// constructor
		Polygon(int, Ogre::SceneNode**); /// This class is <NOT responsible> for the <deletion> of passed nodes.

		virtual ~Polygon(); /// Clean-up memory reserved for caching

		virtual bool isReady();
		virtual void virtualCacheParams();

		inline Ogre::Vector3 getWorldNormal() const { return worldNormal; };

		/// I am a 'Polygon'! 'this' is a 'Polygon*'!! Mr.object, check for collision with a 'Polygon' (me)!!!
		#include "checkCollision.h"

		#include "distance.h"

		virtual double distance(const Ogre::Vector3&) const;
	};

}

#endif