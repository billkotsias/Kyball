// -----------------------------------
// Aim - Aiming aid for BallAI objects
// -----------------------------------
// NOTES : currently supports spheres only : it'll need a rewrite for ANY collision object, and collision objects will have to implement a "copy()" (see CollisionObject.h)

#pragma once
#ifndef Aim_H
#define Aim_H

#include "ShadowCaster.h"
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreColourValue.h>
#include <vector>

namespace Ogre {
	class BillboardSet;
	class SceneNode;
}

namespace P3D {

	class GameplayBase;
	class GameplayObject;
	class Sphere;

	class Aim : public ShadowCaster {

	private:

		static const double POINTS_BEHIND;	/// number of points that are shown <behind> the origin's position
		static const double SHADOW_SIZE;
		static const double NORMAL_SIZE;
		static const double REFLECT_SIZE;
		static const Ogre::ColourValue REFLECT_COL;

		GameplayBase* base;

		/* points geometry */;
		double count;					/// [0,1) - points animation count
		Ogre::SceneNode* sphereNode;	/// "floating" collision-object position
		Ogre::SceneNode* sphereVel;		/// "floating" collision-object velocity
		GameplayObject* sphereObj;		/// gameplay object encapsulating the collision-one
		Ogre::Vector3 startPosition;	/// points start at this <local> position

		/* depiction of points */;
		Ogre::BillboardSet* bbset;
		double countStep;				/// points animation speed
		Ogre::Vector2 defaultSize;
		Ogre::BillboardSet* helpbbset;	/// additional help

		/// make billboard visible
		/// =>	billboard index
		///		position
		///		radius
		///		alpha (affects size?)
		void drawBillboard(unsigned int, const Ogre::Vector3&, const double&, const Ogre::ColourValue&);

		/// shadows
		std::vector<Ogre::Billboard*> pointsDrawn; /// store billboards drawn to add a shadow to them

	public:

		// constructor
		Aim(GameplayBase*, int = 10);
		virtual ~Aim();

		bool enabled;
		double radiusScaler;

		// set "length" of aim
		void setPointsNum(int);
		int getPointsNum();

		// - reset collision object parameters
		// => radius
		//	  mass
		void setPointParams(double, double);

		// run
		// => cycles
		//	  initial velocity
		void run(int, Ogre::Vector3);

		// override from 'ShadowCaster'
		bool updateShadow(Ogre::Vector2*, Ogre::Vector2*);
	};

}

#endif