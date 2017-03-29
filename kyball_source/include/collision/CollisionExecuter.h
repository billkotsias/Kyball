////////////////////
// CollisionExecuter - the actual math implementations of the various collision case-scenarios
////////////////////

#pragma once
#ifndef CollisionExecuter_H
#define CollisionExecuter_H

#include "math\Math.h"
#include "CollisionParams.h"

#include <OgreVector3.h>

namespace P3D {

	class Sphere;
	class Line;
	class Point;
	class Polygon;
	class Cylinder;
	class Cone;

	class CollisionExecuter {

	public:

		// collision detection : the base of all gameplay logic
		// => 2 'CollisionObject's
		// <= NaN		-> collision detection does not occur in the area of interest, or is not implemented for this collision scenario
		//	  double	-> marks at which fraction of the current frame the collision occurs
		static CollisionParams checkCollision(Sphere*, Sphere*);
		static CollisionParams checkCollision(Sphere*, Line*);
		static CollisionParams checkCollision(Sphere*, Point*);
		static CollisionParams checkCollision(Sphere*, Polygon*);
		static CollisionParams checkCollision(Sphere*, Cylinder*);
		static CollisionParams checkCollision(Sphere*, Cone*);


		// static intersection detection
		// => 2 'CollisionObject's
		// <= distance of objects
		static double distance(const Sphere*, const Sphere*);
		static double distance(const Sphere*, const Line*);
		static double distance(const Sphere*, const Point*);
		static double distance(const Sphere*, const Polygon*);
		static double distance(const Sphere*, const Cylinder*);
		static double distance(const Sphere*, const Cone*);


		// reflect a "velocity"-node according to 'CollisionParams' and velocity-owner's mass
		static void reflectVelocity(Ogre::SceneNode*, const CollisionParams&, double);


		//
		// low-level functions
		//

		// ---------------
		// basic distances
		// ---------------

		// point-to-line
		// => point, defined as a vector
		//	  line, defined as 2 vectors
		static double pointLineDistance(const Ogre::Vector3&, const Ogre::Vector3&, const Ogre::Vector3&);

		// point-to-polygon
		// => point
		//	  number of polygon points
		//	  pointer to points
		//	  polygon normal
		static double pointPolygonDistance(const Ogre::Vector3&, const int, const Ogre::Vector3*, const Ogre::Vector3&);

		// ------------------
		// basic intersection
		// ------------------

		// sphere VS line
		// - sphere center is at (0,0,0), with radius r
		// - line has starting point (x0, y0, z0) and direction (ux, uy, uz) ( = speed = translation/iteration)
		// => start	= (x0, y0, z0)
		//	  dir	= (ux, uy, uz)
		//	  r
		//	  optimize? (= don't care for t > 1)
		// <= t		= time of intersection
		//	  norm	= surface normal of intersection point
		static void sphereVSline(const Ogre::Vector3&, const Ogre::Vector3&, const double, const bool, double&, Ogre::Vector3&);

		// cylinder VS line
		// - cylinder (A, B, r) (start point, end point, radius)
		// - line has starting point (x0, y0, z0) and direction (ux, uy, uz) ( = speed = translation/iteration)
		// => start	= (x0, y0, z0)
		//	  dir	= (ux, uy, uz)
		//	  A
		//	  B
		//	  r
		//	  optimize? (= don't care for t > 1)
		// <= t		= time of intersection
		//	  norm	= surface normal of intersection point
		static void cylinderVSline(const Ogre::Vector3&, const Ogre::Vector3&, const Ogre::Vector3&, const Ogre::Vector3&, const double, const bool, double&, Ogre::Vector3&);

		// polygon VS line
		// - polygon consists of consecutive points; it's normal and "normal's best axis" must be provided
		// - line has starting point (x0, y0, z0) and direction (ux, uy, uz) ( = speed = translation/iteration)
		// => start	= (x0, y0, z0)
		//	  dir	= (ux, uy, uz)
		//	  pointsNum = number of polygon points
		//	  points = pointer to points pointers
		//	  normal = polygon normal
		//	  dirNormal = dir.dotProduct(normal)
		//	  optimize? (= don't care for t > 1)
		// <= t		= time of intersection
		//	  surface normal is not returned in this function, as it's the precalculated polygon's normal
		static void polygonVSline(const Ogre::Vector3&, const Ogre::Vector3&, const int, const Ogre::Vector3*,
								  const Ogre::Vector3&, const double, const bool, double&);
		/// helping functions
		static double getLineSide(int, int, const Ogre::Vector3*, const Ogre::Vector3*, const Ogre::Vector3*);
		static void getBestPlaneAxis(const Ogre::Vector3&, int&, int&);

		/* is point inside polygon? */;
		/// => point in question
		///	   polygon points number
		///	   pointer to polygon points
		///	   polygon normal
		static bool pointInPolygon(const Ogre::Vector3&, const int, const Ogre::Vector3*, const Ogre::Vector3&);
	};
}

#endif