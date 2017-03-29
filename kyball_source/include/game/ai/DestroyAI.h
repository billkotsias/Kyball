////////////
// DestroyAI - every object with "infinite mass" which simply makes the balls bounce!
////////////

#pragma once
#ifndef DestroyAI_H
#define DestroyAI_H

#include "AIPosVel.h"

namespace P3D {

	class DestroyAI : public AIPosVel {

	public:

		// constructor
		DestroyAI(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&, bool mover);

		// overrides
		virtual Action* getAction(); /// all objects are destroyed on contact with a "container" object
		virtual CheckForCollision checkForCollision() { return AI::ACCEPT;};

	};
}

#endif