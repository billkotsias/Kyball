///////////
// StickyAI - objects with infinite mass where balls "stick" firmly to
///////////

#pragma once
#ifndef StickyAI_H
#define StickyAI_H

#include "AIPosVel.h"

namespace P3D {

	class StickyAI : public AIPosVel {

	public:

		StickyAI(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&, bool mover);

		// overrides
		virtual Action* getAction();
		virtual CheckForCollision checkForCollision() { return AI::ACCEPT;};
		//virtual bool attachForm(Form*);
	};
}

#endif