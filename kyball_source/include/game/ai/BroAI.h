/////////
// BroAI
/////////

#pragma once
#ifndef BroAI_H
#define BroAI_H

#include "BallAI.h"

namespace P3D {

	class BroAI : public BallAI {

	protected:

		/// overrides
		virtual void onDestroy(Action*);
		virtual void changeInternalState(InternalState);

	public:

		// constructor
		// => 'GameplayObject' "owner" of this AI
		//	  parent 'SceneNode'
		//	  position relative to parent
		BroAI(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&);
		virtual ~BroAI() {};

	};
}

#endif