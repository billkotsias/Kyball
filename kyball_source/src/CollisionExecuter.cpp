#include "StdAfx.h"
#include "collision\CollisionExecuter.h"

#include "collision\Sphere.h"
#include "collision\Cylinder.h"
#include "collision\Polygon.h"
#include "collision\Point.h"
#include "collision\Line.h"

#include "OgreAddOns.h"

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreSceneNode.h>

namespace P3D {

	//
	// Low level functions : intersection cases
	//

	/// Line VS <cylinder>
	void CollisionExecuter::cylinderVSline(const Ogre::Vector3& start, const Ogre::Vector3& dir, const Ogre::Vector3& A, const Ogre::Vector3& B, const double r,
										   const bool optimize, double& t, Ogre::Vector3& normal) {
		t = NaN;

		// Solution : http://www.gamedev.net/community/forums/topic.asp?topic_id=467789
		double cxmin, cymin, czmin, cxmax, cymax, czmax;
		if (A.z < B.z) { czmin = A.z - r; czmax = B.z + r; } else { czmin = B.z - r; czmax = A.z + r; }
		if (A.y < B.y) { cymin = A.y - r; cymax = B.y + r; } else { cymin = B.y - r; cymax = A.y + r; }
		if (A.x < B.x) { cxmin = A.x - r; cxmax = B.x + r; } else { cxmin = B.x - r; cxmax = A.x + r; }
		if (optimize) {
			if (start.z >= czmax && (start.z + dir.z) > czmax) return;
			if (start.z <= czmin && (start.z + dir.z) < czmin) return;
			if (start.y >= cymax && (start.y + dir.y) > cymax) return;
			if (start.y <= cymin && (start.y + dir.y) < cymin) return;
			if (start.x >= cxmax && (start.x + dir.x) > cxmax) return;
			if (start.x <= cxmin && (start.x + dir.x) < cxmin) return;
		}

		Ogre::Vector3 AB = B - A;
		Ogre::Vector3 AO = start - A;
		Ogre::Vector3 AOxAB = AO.crossProduct(AB);
		Ogre::Vector3 VxAB  = dir.crossProduct(AB);
		double ab2	= AB.dotProduct(AB);
		double a	= VxAB.dotProduct(VxAB);
		double b	= 2 * VxAB.dotProduct(AOxAB);
		double c	= AOxAB.dotProduct(AOxAB) - (r*r * ab2);
		double d	= b * b - 4 * a * c;
		if (d < 0) return;
		double time = (-b - sqrt(d)) / (2 * a);
		if (time < 0) return;

		Ogre::Vector3 intersection	= start + dir * time;								/// intersection point
		Ogre::Vector3 projection	= A + (AB.dotProduct(intersection - A) / ab2) * AB;	/// intersection projected onto cylinder axis
		//if ((projection - A).length() + (B - projection).length() > AB.length()) return; /// THIS IS THE SLOW SAFE WAY
		double rt = r * 0.9999; /// error-tolerance!
		if (projection.z > czmax - rt || projection.z < czmin + rt ||
			projection.y > cymax - rt || projection.y < cymin + rt ||
			projection.x > cxmax - rt || projection.x < cxmin + rt ) return; /// THIS IS THE FASTER BUGGY WAY

		normal = (intersection - projection);
		normal.normalise();
		t = time; /// at last
	}

	/// Line VS <sphere>
	void CollisionExecuter::sphereVSline(const Ogre::Vector3& start, const Ogre::Vector3& dir, const double r, const bool optimize,
										 double& t, Ogre::Vector3& normal) {
		t = NaN;

		if (optimize) { /// AABB check (fast)
			if (start.z <= -r	&& (dir.z <= 0 || (start.z + dir.z) < -r))	return;
			if (start.z >= r	&& (dir.z >= 0 || (start.z + dir.z) > r))	return;
			if (start.y <= -r	&& (dir.y <= 0 || (start.y + dir.y) < -r))	return;
			if (start.y >= r	&& (dir.y >= 0 || (start.y + dir.y) > r))	return;
			if (start.x <= -r	&& (dir.x <= 0 || (start.x + dir.x) < -r))	return;
			if (start.x >= r	&& (dir.x >= 0 || (start.x + dir.x) > r))	return;

			/// couldn't detect collision when : (start.z < -r) && (start.z + dir.z == -r) !!!!!
			//if (start.z <= -r	&& (start.z + dir.z) <= -r)	return;
			//if (start.z >= r	&& (start.z + dir.z) >= r)	return;
			//if (start.y <= -r	&& (start.y + dir.y) <= -r)	return;
			//if (start.y >= r	&& (start.y + dir.y) >= r)	return;
			//if (start.x <= -r	&& (start.x + dir.x) <= -r)	return;
			//if (start.x >= r	&& (start.x + dir.x) >= r)	return;
		}

		double c = start.squaredLength() - r * r;
		if (c < 0) return;	/// the 2 spheres initially occupy common space; collision occurs but not in the area of interest
		/// if c == 0, we must check if balls move closer (t = 0) or apart (t = NaN)

		double a = dir.squaredLength();
		if (a == 0) return;	/// the 2 spheres move in parallel

		double b = -2 * start.dotProduct(dir); /// = -2 * start.x * dir. x + start.y * dir.y + start.z * dir.z

		double d = b * b - 4 * a * c;
		if (d <= 0) return; /// no collision or "tangent" collision

		/// calculate collision <time>
		/// - the above restrictions <should assert> that time >= 0. I'll put an exception here to check for some time (pun)...
		/// was : double time = (b - sqrt(d)) / (2 * a); /// <NOT ENOUGH PRECISION> <!!!>
		double a2 = 2 * a;
		double time = b / a2 - sqrt(d / (a2 * a2));
		/// NOTE : for some reason that I need to check some time, it is <NOT> asserted that 'time >= 0'. Hence the check below...
		if (time < 0) return;

		/// calculate collision <normal>
		/// - find collision point
		normal = start + dir * time; /// this point is on sphere with center (0,0,0), so it's the sphere's (and surface's) normal!
		normal.normalise();
		t = time;
	}

	/// Line VS <polygon>
	void CollisionExecuter::polygonVSline(const Ogre::Vector3& start, const Ogre::Vector3& dir, const int pointsNum, const Ogre::Vector3* points,
										  const Ogre::Vector3& normal, const double dirNormal, const bool optimize, double& t) {
		t = NaN;

		/// Solution : http://en.wikipedia.org/wiki/Line-plane_intersection
		/// additional <optimization> :
		/// have a pre-calced <AABB of polygon> and check against it!
		if (dirNormal == 0) return;
		double time = (points[0] - start).dotProduct(normal) / dirNormal; /// one-liner!
		if (time < 0 || (optimize && time > 1)) return;

		/// check if intersection is inside <convex> polygon
		Ogre::Vector3 intersection	= start + dir * time;
		if (!pointInPolygon(intersection, pointsNum, points, normal)) return;

		t = time; /// at last
		
	}

	bool CollisionExecuter::pointInPolygon(const Ogre::Vector3& point, const int pointsNum, const Ogre::Vector3* points, const Ogre::Vector3& normal) {

		/// Solution : http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/ [Solution 3 (2D)]
		int xvar, yvar;
		getBestPlaneAxis(normal, xvar, yvar);
		double sign = getLineSide(xvar, yvar, &point, &points[pointsNum - 1], &points[0]);
		if (sign == 0) return false; /// if point is on polygon <edge>, it's counted <out> (because if p0 == p1, it always results as "in"!)

		for (int i = pointsNum - 1; i >= 1; --i) {
			if (sign * getLineSide(xvar, yvar, &point, &points[i - 1], &points[i]) <= 0) return false;
		}

		return true;
	}

	double CollisionExecuter::getLineSide(int xvar, int yvar, const Ogre::Vector3* p, const Ogre::Vector3* p0, const Ogre::Vector3* p1) {
		double x = *(double*)((int)p + xvar);
		double y = *(double*)((int)p + yvar);
		double x0 = *(double*)((int)p0 + xvar);
		double y0 = *(double*)((int)p0 + yvar);
		double x1 = *(double*)((int)p1 + xvar);
		double y1 = *(double*)((int)p1 + yvar);
		return (y - y0) * (x1 - x0) - (x - x0) * (y1 - y0);
	}

	void CollisionExecuter::getBestPlaneAxis(const Ogre::Vector3& vector, int& xvar, int& yvar) {
		if (abs(vector.x) > abs(vector.y) && abs(vector.x) > abs(vector.z)) {
			xvar = (int)&(vector.y) - (int)&vector;
			yvar = (int)&(vector.z) - (int)&vector;
		} else if (abs(vector.y) > abs(vector.z)) {
			xvar = (int)&(vector.x) - (int)&vector;
			yvar = (int)&(vector.z) - (int)&vector;
		} else {
			xvar = (int)&(vector.x) - (int)&vector;
			yvar = (int)&(vector.y) - (int)&vector;
		}
	}

	//
	// High level functions : collision detection
	//

	/// Sphere VS <Sphere>
	CollisionParams CollisionExecuter::checkCollision(Sphere* sphere1, Sphere* sphere2) {

		/// WATCH IT : CollisionExecuter expects passed objects to be <Ready> !

		/// convert collision to line VS sphere
		/// - calc new sphere radius, position = (0,0,0)
		double sphereRadius = fabs(sphere1->getWorldRadius() + sphere2->getWorldRadius()); /// use |abs| in case of <negative> ("inner") radius
		/// - calc new velocity
		Ogre::Vector3 start = sphere1->getWorldPosition() - sphere2->getWorldPosition();
		Ogre::Vector3 velocity = sphere1->getWorldVelocity() - sphere2->getWorldVelocity();

		double time;
		Ogre::Vector3 normal;
		sphereVSline(start, velocity, sphereRadius, !(sphere1->checkFutureCollision || sphere2->checkFutureCollision), time, normal);

		return CollisionParams(sphere1, sphere2, time, normal);
	}

	/// Sphere VS <Cylinder>
	CollisionParams CollisionExecuter::checkCollision(Sphere* sphere, Cylinder* cylinder) {

		/// convert collision to line VS cylinder
		/// - calc new sphere radius, position = (0,0,0)
		double cylinderRadius = fabs(sphere->getWorldRadius() + cylinder->getWorldRadius()); /// use |abs| in case of <negative> ("inner") radius
		/// - calc new velocity
		Ogre::Vector3 start = sphere->getWorldPosition();
		Ogre::Vector3 velocity = sphere->getWorldVelocity() - cylinder->getWorldVelocity();

		double time;
		Ogre::Vector3 normal;
		cylinderVSline(start, velocity,
					   cylinder->getWorldPoint(0), cylinder->getWorldPoint(1), cylinderRadius,
					   !(sphere->checkFutureCollision || cylinder->checkFutureCollision),
					   time, normal);

		if (time == time) {
			std::cout<<"CollisionExecuter::checkCollision time == time\n";
		}
		return CollisionParams(sphere, cylinder, time, normal);
	}

	/// Sphere VS <Polygon>
	CollisionParams CollisionExecuter::checkCollision(Sphere* sphere, Polygon* poly) {
		Ogre::Vector3 start = sphere->getWorldPosition();
		Ogre::Vector3 dir = sphere->getWorldVelocity() - poly->getWorldVelocity();

		Ogre::Vector3* points = poly->cachedPoints;
		double dirNormal;
		Ogre::Vector3 adder;
		double time;

		dirNormal = dir.dotProduct(poly->getWorldNormal());
		adder = poly->getWorldNormal() * sphere->getWorldRadius();
		if (dirNormal > 0) adder = - adder; /// <watch it> : polygons are expected to be defined <clockwise>

		Ogre::Vector3* finPoints = new Ogre::Vector3[poly->pointsNum];
		for (int i = poly->pointsNum - 1; i >= 0; --i) {
			finPoints[i] = points[i] + adder;
		}

		polygonVSline(start, dir, poly->pointsNum, finPoints, poly->getWorldNormal(), dirNormal, !(sphere->checkFutureCollision || poly->checkFutureCollision), time);

		delete [] finPoints; /// <!!!>

		return CollisionParams(sphere, poly, time, poly->getWorldNormal());
	}

	/// Sphere VS <Point>
	CollisionParams CollisionExecuter::checkCollision(P3D::Sphere *sphere, P3D::Point *point) {

		/// convert collision to line VS sphere

		/// - calc new velocity
		Ogre::Vector3 start = sphere->getWorldPosition() - point->getWorldPosition();
		Ogre::Vector3 velocity = sphere->getWorldVelocity() - point->getWorldVelocity();

		double time;
		Ogre::Vector3 normal;
		sphereVSline(start, velocity, sphere->getWorldRadius(), !(sphere->checkFutureCollision || point->checkFutureCollision), time, normal);

		return CollisionParams(sphere, point, time, normal);
	}

	/// Sphere VS <Line>
	CollisionParams CollisionExecuter::checkCollision(P3D::Sphere *sphere, P3D::Line *line) {

		/// convert collision to line VS cylinder
		/// - calc new velocity
		Ogre::Vector3 start = sphere->getWorldPosition();
		Ogre::Vector3 velocity = sphere->getWorldVelocity() - line->getWorldVelocity();

		double time;
		Ogre::Vector3 normal;
		cylinderVSline(start, velocity,
					   line->getWorldPoint(0), line->getWorldPoint(1), sphere->getWorldRadius(),
					   !(sphere->checkFutureCollision || line->checkFutureCollision),
					   time, normal);

		return CollisionParams(sphere, line, time, normal);
	}


	//
	// High level functions : distance calculus
	//
	
	/// Sphere VS <Sphere>
	double CollisionExecuter::distance(const P3D::Sphere *sphere1, const P3D::Sphere *sphere2) {

		Ogre::Vector3 pos1 = sphere1->getWorldPosition();
		Ogre::Vector3 pos2 = sphere2->getWorldPosition();

		return pos1.distance(pos2) - sphere1->getWorldRadius() - sphere2->getWorldRadius(); /// this is compatible with <negative> radii <!!!>
	}
	
	/// Sphere VS <Point>
	double CollisionExecuter::distance(const P3D::Sphere *sphere, const P3D::Point *point) {

		Ogre::Vector3 pos1 = sphere->getWorldPosition();
		Ogre::Vector3 pos2 = point->getWorldPosition();

		return pos1.squaredDistance(pos2) - sphere->getWorldRadius();
	}
	
	/// Sphere VS <Line>
	double CollisionExecuter::distance(const P3D::Sphere *sphere, const P3D::Line *line) {

		Ogre::Vector3 x1 = line->getWorldPoint(0);
		Ogre::Vector3 x2 = line->getWorldPoint(1);
		Ogre::Vector3 x0 = sphere->getWorldPosition();

		return pointLineDistance(x0, x1, x2) - sphere->getWorldRadius();
	}
	
	/// Sphere VS <Cylinder>; For simplicity, Cylinder is simplified into a <Capsule> !!!
	double CollisionExecuter::distance(const P3D::Sphere *sphere, const P3D::Cylinder *cylinder) {

		Ogre::Vector3 x1 = cylinder->getWorldPoint(0);
		Ogre::Vector3 x2 = cylinder->getWorldPoint(1);
		Ogre::Vector3 x0 = sphere->getWorldPosition();

		return pointLineDistance(x0, x1, x2) - sphere->getWorldRadius() - cylinder->getWorldRadius();
	}
	
	/// Sphere VS <Polygon>
	double CollisionExecuter::distance(const P3D::Sphere *sphere, const P3D::Polygon *polygon) {

		return pointPolygonDistance(sphere->getWorldPosition(), polygon->pointsNum, polygon->cachedPoints, polygon->getWorldNormal()) - sphere->getWorldRadius();
	}

	//
	// Low level functions : distance calculus
	//

	double CollisionExecuter::pointPolygonDistance(const Ogre::Vector3& x0, const int pointsNum, const Ogre::Vector3* points, const Ogre::Vector3& normal) {

		double distance;

		/// Solution : http://mathworld.wolfram.com/Point-PlaneDistance.html

		distance = abs(normal.dotProduct(x0 - points[0]));

		/// - find point of perpendicular on polygon plane
		Ogre::Vector3 p = normal * distance + x0;

		/// - is point inside polygon?
		if (pointInPolygon(p, pointsNum, points, normal)) return distance;

		/// - point not inside polygon; return minimum distance to a 'Polygon's edge
		distance = Infinite;
		for (int i = pointsNum - 1; i >= 1; --i) {
			double edgeDistance = pointLineDistance(x0, points[i-1], points[i]);
			if (edgeDistance < distance) distance = edgeDistance;
		}
		double edgeDistance = pointLineDistance(x0, points[pointsNum - 1], points[0]);
		if (edgeDistance < distance) distance = edgeDistance;

		return distance;
	}

	double CollisionExecuter::pointLineDistance(const Ogre::Vector3 &x0, const Ogre::Vector3 &x1, const Ogre::Vector3 &x2) {
		
		/// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

		/// - check if point-to-line perpendicular lies within line segment

		Ogre::Vector3 x0x1 = x0 - x1;
		double x2x1abs = (x2 - x1).length();
		double t = ( (x0x1).dotProduct(x2 - x1) ) / (x2x1abs * x2x1abs);

		/// - not within line segment; <NOTE> : Cylinder simplified into a <Capsule> cases
		if (t <= 0) {
			return x0.distance(x1);
		} else if (t >= 1) {
			return x0.distance(x2);
		}

		/// - within line segment
		return ( x0x1.crossProduct(x0 - x2) ).length() / x2x1abs;
	}


	//

	void CollisionExecuter::reflectVelocity(Ogre::SceneNode* vel, const CollisionParams& collisionParams, double mass) {

		/// get velocity in world co-ords
		Ogre::Vector3 parentSystem = OgreAddOns::getDerivedPosition(vel->getParentSceneNode());
		Ogre::Vector3 velocity = OgreAddOns::getDerivedPosition(vel) - parentSystem;

		/// reflect velocity depending on CollisionParams - <lambda> is assumed to have been <calculated> by this point!
		velocity = velocity - collisionParams.lambda / mass * collisionParams.normal;

		velocity += parentSystem;

		/// set new velocity
		OgreAddOns::setDerivedPosition(vel, &velocity);
	}
}