///////////
// Animator Rotator
///////////

#pragma once
#ifndef AnimatorRotator_H
#define AnimatorRotator_H

#include "visual\Visual.h"

namespace P3D {

	class AnimatorRotator : public Visual {

	protected:

		Ogre::SceneNode* node;
		Ogre::Vector3 axis;
		double sp;

	public:

		AnimatorRotator(Ogre::SceneNode*, const Ogre::Vector3& _axis, double speed, double start);

		virtual bool run(double);
	};
}

#endif