/////////////////////////////////////
// AnimatedEntity - Animation Wrapper
/////////////////////////////////////

#pragma once
#ifndef ASWDelayed_H
#define ASWDelayed_H

#include "AnimationStateWrapper.h"

namespace P3D {

	class ASWDelayed : public AnimationStateWrapper {

	public:

		ASWDelayed(Ogre::AnimationState* _anim, double _speed = 1., double _delay = 0.);
		virtual ~ASWDelayed() {};

		double delay;
		double timeToStart;

		virtual void reset();
		virtual double run(double time);
	};
}

#endif