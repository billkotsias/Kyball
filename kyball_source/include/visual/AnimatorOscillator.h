///////////
// Animator Oscillator
///////////

#pragma once
#ifndef AnimatorOscillator_H
#define AnimatorOscillator_H

#include "visual\Visual.h"

namespace P3D {

	class AnimatorOscillator : public Visual {

	protected:

		Ogre::SceneNode* node;
		Ogre::Vector3 axis;		/// oscillation axis
		double wi;				/// "width", aka "amplitude"
		double sp;
		double t;
		Ogre::Vector3 quant;	/// last added quantity
		bool localTransform;

	public:

		AnimatorOscillator(Ogre::SceneNode*, const Ogre::Vector3& _axis, double amplitude, bool _localTransform, double speed, double start);

		virtual bool run(double);
	};
}

#endif