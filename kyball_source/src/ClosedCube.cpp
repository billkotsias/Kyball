#include "StdAfx.h"
#include "game\form\ClosedCube.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "collision\Cylinder.h"
#include "collision\Polygon.h"
#include "collision\Sphere.h"
#include "collision\Complex.h"
#include "game\ai\AI.h"

#include "tween\TLinear.h"
#include "tween\TCubeFX.h"
#include "tween\TPlayer.h"

#include "OgreAddOns.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>

namespace P3D {

	ClosedCube::ClosedCube(std::string name, std::string material, double x, double y, double z, bool visible, double fix) {

		static const double X0 = 0.;
		static const double X1 = -1.;
		static const double YB = 0.;	/// y-bottom
		static const double YT = 1.;	/// y-top
		static const double Z0 = 0.;
		static const double Z1 = 1.;

		pivot->translate(fix,-fix,-fix);
		pivot->setScale(x, y, z);

		/// build <collisionObject>
		Ogre::SceneNode* p0 = newSceneNode(pivot, &Ogre::Vector3(X0, YB, Z0));
		Ogre::SceneNode* p1 = newSceneNode(pivot, &Ogre::Vector3(X1, YB, Z0));
		Ogre::SceneNode* p2 = newSceneNode(pivot, &Ogre::Vector3(X1, YT, Z0));
		Ogre::SceneNode* p3 = newSceneNode(pivot, &Ogre::Vector3(X0, YT, Z0));
		Ogre::SceneNode* p4 = newSceneNode(pivot, &Ogre::Vector3(X1, YB, Z1));
		Ogre::SceneNode* p5 = newSceneNode(pivot, &Ogre::Vector3(X0, YB, Z1));
		Ogre::SceneNode* p6 = newSceneNode(pivot, &Ogre::Vector3(X0, YT, Z1));
		Ogre::SceneNode* p7 = newSceneNode(pivot, &Ogre::Vector3(X1, YT, Z1));

		sideA[0] = p5; /// Right
		sideA[1] = p0;
		sideA[2] = p3;
		sideA[3] = p6;

		sideB[0] = p1; /// Left
		sideB[1] = p4;
		sideB[2] = p7;	
		sideB[3] = p2;

		sideC[0] = p2; /// Top
		sideC[1] = p7;
		sideC[2] = p6;
		sideC[3] = p3;

		sideD[0] = p0; /// Bottom
		sideD[1] = p5;
		sideD[2] = p4;
		sideD[3] = p1;

		sideE[0] = p3; /// Front
		sideE[1] = p2;
		sideE[2] = p1;
		sideE[3] = p0;

		sideF[0] = p4; /// Back
		sideF[1] = p5;
		sideF[2] = p6;
		sideF[3] = p7;

		Complex* complex = new Complex(); /// <!!!> 26 sub-objects <!!!>
		complex->addObject(new Polygon(4, sideA));
		complex->addObject(new Polygon(4, sideB));
		complex->addObject(new Polygon(4, sideC));
		complex->addObject(new Polygon(4, sideD));
		complex->addObject(new Polygon(4, sideE));
		complex->addObject(new Polygon(4, sideF));

		complex->addObject(new Cylinder(p0, p1, 0));
		complex->addObject(new Cylinder(p1, p2, 0));
		complex->addObject(new Cylinder(p2, p3, 0));
		complex->addObject(new Cylinder(p3, p0, 0));

		complex->addObject(new Cylinder(p4, p5, 0));
		complex->addObject(new Cylinder(p5, p6, 0));
		complex->addObject(new Cylinder(p6, p7, 0));
		complex->addObject(new Cylinder(p7, p4, 0));

		complex->addObject(new Cylinder(p3, p6, 0));
		complex->addObject(new Cylinder(p0, p5, 0));
		complex->addObject(new Cylinder(p2, p7, 0));
		complex->addObject(new Cylinder(p1, p4, 0));

		complex->addObject(new Sphere(p0, 0));
		complex->addObject(new Sphere(p1, 0));
		complex->addObject(new Sphere(p2, 0));
		complex->addObject(new Sphere(p3, 0));
		complex->addObject(new Sphere(p4, 0));
		complex->addObject(new Sphere(p5, 0));
		complex->addObject(new Sphere(p6, 0));
		complex->addObject(new Sphere(p7, 0));

		collisionObject = complex;


		/// build <MovableObject>
		if (visible) {
			storeMovable( OgreBuilder::createEntity(name, "closedCube.mesh", false, material, pivot) );
		}
	}

	ClosedCube::~ClosedCube() {
	}
}