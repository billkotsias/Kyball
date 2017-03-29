///////////
// Cylinder
///////////

#pragma once
#ifndef Cylinder_H
#define Cylinder_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"
#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class Cylinder : public CollisionObject {

	protected:

		Ogre::SceneNode* points[2];	/// defining start-end points
		double radius;				/// 'Cylinder's radius; assumed to be a <child> parameter of the <1st (starting) point!>
		bool negativeRadius;		/// if true, Cylinder will be checked for "inner collision" (Cylinder contains other object)

		/// cached params
		double worldRadius;	/// position gets cached by Ogre; but this one is more cpu-intensive

	public:

		// constructor
		// => p1, p2
		//	  radius
		//	  negativeRadius
		Cylinder(Ogre::SceneNode*, Ogre::SceneNode*, double, bool = false); /// This class is <NOT responsible> for the <deletion> of passed nodes.

		/* run-time modifiers */;
		inline void setLocalRadius(double inRad) { radius = inRad; };

		virtual void virtualCacheParams();
		virtual bool isReady();

		Ogre::Vector3 getWorldPoint(unsigned int) const;				/// get a cylinder's point in world co-ordinates
		inline double getWorldRadius() const { return worldRadius;} ;	/// get cylinder's radius in world co-ordinates

		/// I am a 'Cylinder'! 'this' is a 'Cylinder*'!! Mr.object (Sphere!), check for collision with a 'Cylinder' (me)!!!
		#include "checkCollision.h"

		#include "distance.h"

		virtual double distance(const Ogre::Vector3&) const;
	};

}

#endif