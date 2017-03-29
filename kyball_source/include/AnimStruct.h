/////////////
// AnimStruct : REJECTED!!! (advanced animation synthesis, for combining multiple animations at once)
/////////////

#pragma once
#ifndef AnimStruct_H
#define AnimStruct_H

#include <deque>

namespace FANLib {
	class FSLClass;
}

namespace Ogre {
	class AnimationState;
	class Entity;
}

namespace P3D {

	class TSPair {
	public:
		double time;
		double step; /// new = old + time * step
	};

	// Animas are being self-destroyed as they are played (alters pair's time and removes finished ones), so a copy must be played
	class Anima {
	public:
		Ogre::AnimationState* animation;
		double currValue;	/// value at current t
		std::deque<TSPair> pairs;

		void checkEnable();	/// animation will be disabled if value == 0 and 1st pair's step == 0
		bool run(double);	/// has this animation finished?
	};

	// AnimaSet is also being self-destroyed (removes finished Animas)
	class AnimaSet {
	public:

		std::deque<Anima> animas;

		// constructor
		AnimaSet(Ogre::Entity*, FANLib::FSLClass *);

		AnimaSet start();	/// mass-check which animations must be enabled
		bool run(double);	/// <= has this animation set finished?
	};
}

#endif