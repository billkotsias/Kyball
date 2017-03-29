/////////////////////////////////////////////////////////
// Sphere - the most important 'CollisionObject' subclass
/////////////////////////////////////////////////////////

#pragma once
#ifndef Sphere_H
#define Sphere_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"
#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class Sphere : public CollisionObject {

	protected:

		Ogre::SceneNode* pos;		/// 'Sphere's <local> position; must be checked if it's in <sceneGraph>
		double radius;				/// 'Sphere's <local> radius; assumed to be a 'pos' child parameter
		bool negativeRadius;		/// if true, sphere will be checked for "inner collision" (sphere contains other object)

		/// cached params
		double worldRadius;	/// position gets cached by Ogre; but this one is more cpu-intensive

	public:

		// constructor
		// => pos
		//	  radius
		//	  negativeRadius
		Sphere(Ogre::SceneNode*, double, bool = false); /// This class is <NOT responsible> for the <deletion> of passed nodes.

		/* run-time modifiers */;
		inline void setLocalRadius(double inRad) { radius = inRad; };

		virtual void virtualCacheParams();
		virtual bool isReady();

		Ogre::Vector3 getWorldPosition() const;							/// get sphere's position in world co-ordinates
		inline double getWorldRadius() const { return worldRadius;} ;	/// get sphere's radius in world co-ordinates

		/// I am a 'Sphere'! 'this' is a 'Sphere*'!! Mr.object, check for collision with a 'Sphere' (me)!!!
		#include "checkCollision.h"

		virtual CollisionParams checkCollision(Cylinder* cylinder, int TTL) {
			return CollisionExecuter::checkCollision(this, cylinder);
		};

		virtual CollisionParams checkCollision(Line* line, int TTL) {
			return CollisionExecuter::checkCollision(this, line);
		};

		virtual CollisionParams checkCollision(Point* point, int TTL) {
			return CollisionExecuter::checkCollision(this, point);
		};

		virtual CollisionParams checkCollision(Polygon* polygon, int TTL) {
			return CollisionExecuter::checkCollision(this, polygon);
		};


		#include "distance.h"

		virtual double distance(const Cylinder* cylinder, int TTL) const {
			return CollisionExecuter::distance(this, cylinder);
		};

		virtual double distance(const Line* line, int TTL) const {
			return CollisionExecuter::distance(this, line);
		};

		virtual double distance(const Point* point, int TTL) const {
			return CollisionExecuter::distance(this, point);
		};

		virtual double distance(const Polygon* polygon, int TTL) const {
			return CollisionExecuter::distance(this, polygon);
		};

		virtual double distance(const Ogre::Vector3&) const;
	};

}

#endif