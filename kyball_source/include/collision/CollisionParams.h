//////////////////
// CollisionParams
//////////////////

#pragma once
#ifndef CollisionParams_H
#define CollisionParams_H

#include <OgreVector3.h>
#include <math\math.h>

namespace P3D {

	class GameplayObject;
	class CollisionObject;

	class CollisionParams {

	public:

		// properties
		CollisionObject* colObj1;	/// } Colliding objects; may be redundant
		CollisionObject* colObj2;	/// }
		double time;				/// time of collision
		Ogre::Vector3 normal;		/// normal of collision surface

		/* "added" properties */;
		GameplayObject* gameObj1;
		GameplayObject* gameObj2;
		double lambda;


		// constructors
		/// default constructor, when needed (e.g by STL containers)
		inline CollisionParams() {
			colObj1 = 0;
			colObj2 = 0;
			time = NaN;
			normal = Ogre::Vector3::ZERO;
		};
		/// direct constructor
		inline CollisionParams(P3D::CollisionObject *obj1, P3D::CollisionObject *obj2, double t, Ogre::Vector3 norm) {
			colObj1 = obj1;
			colObj2 = obj2;
			time = t;
			normal = norm;
		};

		/// Common params for <no collision>
		static const CollisionParams NO_COLLISION;


		// operators
		inline bool isValid() {
			return (time == time && time > 0); /// OLD NOTE : Must change to 'time >= 0' if I am to allow threesome simultaneous collisions
		};

		inline CollisionParams swap() const {

			CollisionParams swapped(colObj2, colObj1, time, normal);
			swapped.gameObj1 = gameObj2;
			swapped.gameObj2 = gameObj1;
			swapped.lambda = -lambda;

			return swapped;
		};

		inline bool operator<(const CollisionParams &other) const {
			double temp1, temp2;

			temp1 = time;
			if (temp1 != temp1) temp1 = Infinite;
			temp2 = other.time;
			if (temp2 != temp2) temp2 = Infinite;

			if (temp1 != temp2) return (temp1 < temp2);
			return ((int)this < (int)(&other)); /// if they are equal in time, sort them "arbitrarily" by memory address
		};

		inline bool operator==(const CollisionParams &other) const {
			if (
				time == other.time &&
				normal == other.normal &&
				(
					(colObj1 == other.colObj1 && colObj2 == other.colObj2) ||
					(colObj1 == other.colObj2 && colObj2 == other.colObj1)
				)
				)
				return true;

			return false;
		};

		inline bool operator!=(const CollisionParams &other) const {
			return !(*this == other);
		};


		// functions
		void calculateLambda();
	};
}

#endif