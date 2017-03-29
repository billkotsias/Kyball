#pragma once
#ifndef AnimationStateWrapper_H
#define AnimationStateWrapper_H

namespace P3D {

	class AnimationStateWrapper {

	protected:

		Ogre::AnimationState* animState;
		double speed;

	public:

		/// <TODO> Kyball 2 : Add different start positions for each anim! (used in P3DExtro Dragon!!! truely useful with "combined" animations)
		AnimationStateWrapper(Ogre::AnimationState* _anim, double _speed = 1.) : animState(_anim), speed(_speed) {
		}
		virtual ~AnimationStateWrapper() {}

		/// <= returns remaining time NOT consumed by this animation
		virtual void reset() = 0;
		virtual double run(double time) = 0;

		double getLength() const
		{
			return animState->getLength();
		}

		void setEnabled(bool enable) { animState->setEnabled(enable); }
		void setLoop(bool loop) { animState->setLoop(loop); }

#ifdef _DEBUG
		/// debug
		std::string getName() { return animState->getAnimationName(); }
#endif
	};
}

#endif