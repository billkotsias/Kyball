#include "StdAfx.h"
#include "collision\Complex.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>

#include "collision\Sphere.h"
#include "collision\Cylinder.h"
#include "collision\Polygon.h"
#include "collision\Point.h"
#include "collision\Line.h"

#include "math\Math.h"

namespace P3D {

	Complex::~Complex() {
		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			delete it->first;
		}
		objects.clear();
	}

	void Complex::virtualCacheParams() {
		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			it->first->worldVelocity = worldVelocity;	/// } Optimization : 'Complex's 'worldVelocity' is the same for every sub-object
			it->first->virtualCacheParams();			/// }
		}
	}

	bool Complex::isReady() {
		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			if (!(it->first->isReady())) return false;
		}
		return true;
	}

	void Complex::addObject(P3D::CollisionObject *obj) {
		objects[obj] = true;
	}

	void Complex::removeObject(P3D::CollisionObject *obj) {
		objects.erase(obj);
	}

	CollisionParams Complex::checkCollision(P3D::CollisionObject *object, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = object->checkCollision(it->first, TTL); /// don't reduce <TTL> yet
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Sphere* sphere, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = it->first->checkCollision(sphere, --TTL);
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Polygon* polygon, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = it->first->checkCollision(polygon, --TTL);
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Point* point, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = it->first->checkCollision(point, --TTL);
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Cylinder* cylinder, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = it->first->checkCollision(cylinder, TTL);
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Line* line, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = it->first->checkCollision(line, --TTL);
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	CollisionParams Complex::checkCollision(Complex* complex, int TTL) {
		CollisionParams earliest = CollisionParams::NO_COLLISION;

		for (std::map<CollisionObject*, bool>::iterator it = objects.begin(); it != objects.end(); ++it) {
			CollisionParams params = complex->checkCollision(it->first, TTL); /// don't reduce <TTL> yet
			if (!(earliest.time < params.time) && params.time == params.time) {
				earliest = params;
			}
		}
		return earliest;
	}

	//

	double Complex::distance(const P3D::CollisionObject *object, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = object->distance(it->first, TTL); /// don't reduce <TTL> yet
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Sphere* sphere, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = sphere->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Polygon* polygon, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = polygon->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Line* line, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = line->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Point* point, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = point->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Cylinder* cylinder, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = cylinder->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	double Complex::distance(const Complex* complex, int TTL) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = complex->distance(it->first, --TTL);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}

	//

	double Complex::distance(const Ogre::Vector3 &p) const {
		/// return minimum distance from sub-objects
		double distance = Infinite;

		for (std::map<CollisionObject*, bool>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
			double objDistance = it->first->distance(p);
			if (objDistance < distance) distance = objDistance;
		}

		return distance;
	}
}