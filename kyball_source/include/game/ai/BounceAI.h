///////////
// BounceAI - every object with "infinite mass" which simply makes the balls bounce!
///////////

#pragma once
#ifndef BounceAI_H
#define BounceAI_H

#include "AIPosVel.h"

namespace P3D {

	class BounceAI : public AIPosVel {

	public:

		// constructor
		BounceAI(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&, bool mover);

		// overrides
		virtual Action* getAction(); /// all objects are reflected on contact with a "container" object
		virtual CheckForCollision checkForCollision() { return AI::ACCEPT;};

		// old ideas:
		/// <TODO> : make a "visual effect" at the point of contact with another object :
		/// <NOTE> : another effect would be for the cube to "show in advance" the point of contact with a ball (through <checkFutureCollision>)
		virtual void receiveAction(Action*);

	};
}

#endif