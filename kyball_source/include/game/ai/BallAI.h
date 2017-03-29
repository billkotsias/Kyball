/////////
// BallAI
/////////

#pragma once
#ifndef BallAI_H
#define BallAI_H

#include "AIPosVel.h"
#include "AIPVListener.h"
#include "game\net\Net.h"

#include <OgreVector3.h>

namespace P3D {

	class AIPVAnime;

	class BallAI : public AIPosVel, AIPVListener {

	public:

		enum InternalState {
			OUT_OF_PLAY,
			STUCK,
			FLYING,
			FALLING,
			MOVING_IN_PLACE,	/// during "sticking" in-game-animation
		};

	private:

		/* properties altered by 'changeInternalState()' */;
		Action* myAction;				/// behaviour to other objects

	protected:

		CheckForCollision checkCollision;
		InternalState internalState;	/// keep copy of current internal state

		NetNode netStuckPosition; /// set/unset this node's availability

		/// possible reactions to incoming actions :
		virtual void onReflect(Action*);
		virtual void onStick(Action*);
		virtual void onDestroy(Action*);
		virtual void onFall(Action*);

		virtual void prepareToDie();

		// change 'Ball's internal parameters
		virtual void changeInternalState(InternalState);
		virtual void changeAction(Action*);

		friend class Cannon; /// calls 'changeInternalState'

	public:

		static const double TIME_TO_STICK;

		// constructor
		// => 'GameplayObject' "owner" of this AI
		//	  InternalState
		//	  parent 'SceneNode'
		//	  position relative to parent
		//	  mass
		BallAI(GameplayObject*, BallAI::InternalState, Ogre::SceneNode*, const Ogre::Vector3&, double = 1);
		virtual ~BallAI();

		// get 'Ball'-specific parameters
		inline BallAI::InternalState getInternalState() { return internalState; };

		// overrides
		///virtual void endOfCycle();
		virtual CheckForCollision checkForCollision();
		virtual Action* getAction();
		virtual void receiveAction(Action*);
		virtual bool attachForm(Form*);
		virtual void setWorldPosition(const Ogre::Vector3&);
		//virtual void futureAction(Action* action);

		virtual void animeOver(AIPVAnime*);

	};
}

#endif