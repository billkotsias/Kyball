/////////////
// ClosedCube - a 'Form' descendant
/////////////

#pragma once
#ifndef ClosedCube_H
#define ClosedCube_H

#include "Form.h"
#include "BallState.h"
#include "game\GameplayCollection.h"
#include "game\event\EventListener.h"
#include "tween\TPlayed.h"

#include <string>
#include <OgreColourValue.h>

namespace Ogre {
	class SceneNode;
	class Pass;
	class Entity;
}

namespace P3D {

	class ClosedCube : public Form, public GameplayCollection {

	private:

		/// collision object
		Ogre::SceneNode* sideA[4];
		Ogre::SceneNode* sideB[4];
		Ogre::SceneNode* sideC[4];
		Ogre::SceneNode* sideD[4];
		Ogre::SceneNode* sideE[4];
		Ogre::SceneNode* sideF[4];

	public:

		// constructor
		// => name = name to give to the created entity
		//	  material = name of material to use
		//	  x,y,z = width, height, depth of entity
		//	  visible
		//	  fix = added depth (split at the cube's 2 open ends); usually equal to a ball's radius!
		ClosedCube(std::string name, std::string material, double x, double y, double z, bool visible, double fix = BallState::BALL_RADIUS);
		virtual ~ClosedCube();

	};
}

#endif