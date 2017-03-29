/////////
// BobAI
/////////

#pragma once
#ifndef BobAI_H
#define BobAI_H

#include "BallAI.h"

namespace P3D {

	class BobAI : public BallAI {

	protected:

		/// overrides
		virtual void onDestroy(Action*);

	public:

		// constructor
		// => 'GameplayObject' "owner" of this AI
		//	  parent 'SceneNode'
		//	  position relative to parent
		//	  mass
		BobAI(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&, double = 1);
		virtual ~BobAI() {};

	};
}

#endif