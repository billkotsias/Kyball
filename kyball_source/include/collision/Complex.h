//////////
// Complex
//////////

#pragma once
#ifndef Complex_H
#define Complex_H

#include "CollisionObject.h"
#include "CollisionExecuter.h"

#include <map>

namespace P3D {

	class Complex : public CollisionObject {

	protected:

		std::map<CollisionObject*, bool> objects;

	public:

		//virtual void setVelocity(Ogre::SceneNode*); /// ??? Is this needed ???

		void addObject(CollisionObject*);		/// added objects will be deleted on 'Complex's destruction
		void removeObject(CollisionObject*);	/// NOT DELETED ON REMOVAL! DELETE IT <YOURSELF>

		virtual ~Complex();

		virtual void virtualCacheParams();
		virtual bool isReady();

		virtual CollisionParams checkCollision(CollisionObject*, int = 2);

		virtual CollisionParams checkCollision(Sphere* sphere, int);
		virtual CollisionParams checkCollision(Cylinder* cylinder, int);
		virtual CollisionParams checkCollision(Line* line, int);
		virtual CollisionParams checkCollision(Point* point, int);
		virtual CollisionParams checkCollision(Polygon* polygon, int);
		virtual CollisionParams checkCollision(Complex* complex, int);

		virtual double distance(const CollisionObject* object, int TTL = 2) const;
		virtual double distance(const Sphere* sphere,		int TTL) const;
		virtual double distance(const Line* line,			int TTL) const;
		virtual double distance(const Point* point,			int TTL) const;
		virtual double distance(const Polygon* polygon,		int TTL) const;
		virtual double distance(const Cylinder* cylinder,	int TTL) const;
		virtual double distance(const Complex* complex,		int TTL) const;

		virtual double distance(const Ogre::Vector3&) const;
	};
}

#endif