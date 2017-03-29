#include "StdAfx.h"
#include "visual\ASWDelayed.h"

namespace P3D {

	ASWDelayed::ASWDelayed(Ogre::AnimationState* _anim, double _speed, double _delay) : AnimationStateWrapper(_anim, _speed), delay(_delay)
	{
		animState->setLoop(false);
		reset();
	}

	void ASWDelayed::reset() {
		timeToStart = delay;
		if (speed >= 0)
			animState->setTimePosition(0.);
		else
			animState->setTimePosition( animState->getLength() );
	}

	double ASWDelayed::run(double time)
	{
		if (timeToStart > 0) {

			timeToStart -= time;
			if (timeToStart >= 0) return -1; /// not started yet

			time = -timeToStart; /// time-to-run leftovers
		}

		double lastPosition = animState->getTimePosition();
		animState->addTime(time * speed);

		if ( speed >= 0 && animState->hasEnded() ) {
			double timeConsumed = ( animState->getLength() - lastPosition ) / speed;
			return time - timeConsumed;
		} else if ( speed < 0 && animState->getTimePosition() <= 0 ) {
			double timeConsumed = ( -lastPosition ) / speed;
			return time - timeConsumed;
		}

		return -1;
	}
}