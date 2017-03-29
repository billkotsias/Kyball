/////////////////////////////////////////////////////
// CollisionObject - the base of all gameplay objects
/////////////////////////////////////////////////////

#pragma once
#ifndef CollisionObject_H
#define CollisionObject_H

#include <math.h>
#include <math\Math.h>
#include <OgreVector3.h>
#include "CollisionParams.h"

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class Sphere;		/// }
	class Line;			/// }
	class Point;		/// } 'CollisionObject' descendants
	class Polygon;		/// }
	class Cylinder;		/// }
	class Cone;			/// }
	class Complex;		/// } eg. a <Capsule> is a Complex : 2 spheres and a cylinder; also a <Cube> : 8 Points, 6 Polygons, 8 Lines, all sharing 8 SceneNodes

	class CollisionObject {

	protected:

		Ogre::SceneNode* velocity;			/// object's <local> velocity; this is the only common value for all 'CollisionObject's

		void cacheVelocity();
		Ogre::Vector3 worldVelocity;		/// cached world velocity

		friend class Complex;

	public:

		void setVelocity(Ogre::SceneNode*); /* was: virtual */;
		Ogre::SceneNode* getVelocity();

		inline Ogre::Vector3 getWorldVelocity() { return worldVelocity; }; /// get object's cached velocity in world co-ordinates

		bool checkFutureCollision;			/// does object require "full" collision checking? <false> setting may be faster

		// constructor
		CollisionObject();
		virtual ~CollisionObject() {};

		// copy!
		/// - SELF-NOTE : because objects are node-hierarchy <independent>, a copy must create new nodes on the fly. An idea is to get the <parents> of current object's
		///   SceneNodes and create the same <children> nodes; their deletion must then be made by... <WHO??????????> THE CALLER WILL RECEIVE A LIST OF NODES ALONGSIDE HIS
		///   NEWLY FRESH COPY! HA! I CAN'T ADD A VELOCITY TO THESE NODES BECAUSE MAYBE THEY ARE INTERCONNECTED. REJECT THE COPY FOR NOW! A NEW DESIGN IS NEEDED, FOR
		///	  EXAMPLE EVERY COLLISION OBJECT HAS !1! ROOT NODE, SO WE ADD TO IT, BUT THEN THEY WOULDN'T BE SO DYNAMIC..! URGH!

		// is object ready to be checked for collision? (all parameters except velocity must be initialized & in 'sceneGraph')
		/// this must be called prior to <cacheParams()>
		virtual bool isReady() = 0;

		// cache parameters for this collision loop
		void cacheParams();
		virtual void virtualCacheParams() {};


		// 'CollisionObject's handle collision-resolving themselves :
		/* overloaded cases which MUST be implemented by all subclasses */;

		/// - the BODY of this COLLISION RESOLVING function MUST BE EXACTLY THE SAME for all subclasses,
		///	  so : just #include "checkCollisionObject.h" instead of rewriting every time
		virtual CollisionParams checkCollision(CollisionObject* object, int TTL = 2) = 0; /// TimeToLive in case of circular calling due to non-implementation

		/// - this function implements the most basic of all collisions, a 'Sphere' vs any other 'CollisionObject'
		virtual CollisionParams checkCollision(Sphere* sphere, int TTL) = 0;

		/* additional collision cases : */;

		/// - the 'Sphere' class SHOULD implement all of these
		/// - the 'Complex' class should also implement them, but will only support collision with its 'Spheres' (OK if Complex is only consisted of Spheres)
		/// - other classes may implement them in future projects
		virtual CollisionParams checkCollision(Line* line,			int TTL)	{return CollisionParams::NO_COLLISION;};
		virtual CollisionParams checkCollision(Point* point,		int TTL)	{return CollisionParams::NO_COLLISION;};
		virtual CollisionParams checkCollision(Polygon* polygon,	int TTL)	{return CollisionParams::NO_COLLISION;};
		virtual CollisionParams checkCollision(Cylinder* cylinder,	int TTL)	{return CollisionParams::NO_COLLISION;};
		virtual CollisionParams checkCollision(Cone* cone,			int TTL)	{return CollisionParams::NO_COLLISION;};
		virtual CollisionParams checkCollision(Complex* complex,	int TTL)	{return CollisionParams::NO_COLLISION;};


		// 'CollisionObject's handle intersection-resolving themselves :
		virtual double distance(const CollisionObject* object,	int TTL = 2) const = 0;
		virtual double distance(const Sphere* sphere,			int TTL) const = 0;
		virtual double distance(const Line* line,				int TTL) const {return false;};
		virtual double distance(const Point* point,				int TTL) const {return false;};
		virtual double distance(const Polygon* polygon,			int TTL) const {return false;};
		virtual double distance(const Cylinder* cylinder,		int TTL) const {return false;};
		virtual double distance(const Cone* cone,				int TTL) const {return false;};
		virtual double distance(const Complex* complex,			int TTL) const {return false;};


		// distance of a point (in world-coord system) from this 'CollisionObject'
		virtual double distance(const Ogre::Vector3& point) const = 0;
	};
}

#endif