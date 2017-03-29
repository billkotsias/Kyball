////////
// FlyIn - Game level camera fly-in
////////

#pragma once
#ifndef FlyIn_H
#define FlyIn_H

#include "visual\Visual.h"
#include "hid\HIDListener.h"

namespace P3D {

	class Queue;
	class GameplayBase;
	class HintBox;
	class SubMap;

	class FlyIn : public Visual, public HIDListener {

	protected:

		enum Control {
			NOTHING,
			FAST_FORWARD,
			PAUSE,
			REWIND,
			RESET,
		};

		Ogre::AnimationState* anim;
		unsigned int timesShown;
		Control control;

		Ogre::Camera* myCam;
		Ogre::Camera* origCam;

		Queue* myQueue;
		HintBox* hintBox; /// animal "talking" window
		SubMap* skipText;

		GameplayBase* gameplayBase; /// provides access to 'Animal'

	public:

		static const unsigned int MAX_SHOWN;
		static const char* const FLY_IN;
		static const double FAST_FORWARD_MUL;

		FlyIn(GameplayBase*);
		~FlyIn();

		void start(void*);
		void end(void* = 0);

		// overrides
		virtual bool run(double);
		virtual void onMouseDown();
		virtual void onKeyEvent();
	};
}

#endif