#include "StdAfx.h"
#include "visual\AnimatorOscillator.h"
#include "visual\VisualBoss.h"
#include "OgreAddOns.h"

namespace P3D {

	AnimatorOscillator::AnimatorOscillator(Ogre::SceneNode* _node, const Ogre::Vector3& _axis, double amplitude, bool _localTransform, double speed, double start) :
		node(_node), wi(amplitude), sp(speed), axis(_axis), quant(Ogre::Vector3::ZERO), localTransform(_localTransform)
	{
		t = (start == start) ? start * Ogre::Math::PI / 180. : rand() * Ogre::Math::TWO_PI / (double)RAND_MAX;

		VisualBoss::getSingletonPtr()->insert(this);
	}

	bool AnimatorOscillator::run(double time) {

		t += time * sp;
		while (t >= Ogre::Math::TWO_PI) { t -= Ogre::Math::TWO_PI; };

		Ogre::Vector3 pos = localTransform ? node->getPosition() : OgreAddOns::getDerivedPosition(node);

		pos -= quant; /// remove previous quantity before adding new one
		quant = wi * sin(t) * axis;
		pos += quant;

		if (localTransform)
			node->setPosition(pos);
		else
			OgreAddOns::setDerivedPosition(node, &pos);

		return false;
	}
}