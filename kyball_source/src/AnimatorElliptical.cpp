#include "StdAfx.h"
#include "visual\AnimatorElliptical.h"
#include "visual\VisualBoss.h"
#include "OgreAddOns.h"

namespace P3D {

	AnimatorElliptical::AnimatorElliptical(Ogre::SceneNode* node, double major, double minor, double angle, double rotation, double speed, double start) : ma(major), mi(minor), rot(rotation), sp(speed) {

		if (start == start) {
			start *= Ogre::Math::PI / 180.;
		} else {
			start = rand() * Ogre::Math::TWO_PI / (double)RAND_MAX;
		}

		fi = angle * Ogre::Math::PI/180.;

		const Ogre::Vector3& pos = OgreAddOns::getDerivedPosition(node);
		xc = pos.x;
		zc = pos.z;

		for (unsigned int i = 0; i < node->numChildren(); ++i) {
			Ogre::SceneNode* child = (Ogre::SceneNode*)node->getChild(i);
			const Ogre::Vector3& cPos = OgreAddOns::getDerivedPosition(child);
			double t = atan2( (cPos.z - zc) * ma, (cPos.x - xc) * mi ) + start; /// set starting "time" position for each node
			nodes.push_back( std::pair<Ogre::SceneNode*, double>(child, t) );
		}

		VisualBoss::getSingletonPtr()->insert(this);
	}

	bool AnimatorElliptical::run(double time)
	{
		time *= sp;

		for (int i = nodes.size() - 1; i >= 0; --i) {

			double& t = nodes.at(i).second;
			t += time;
			while (t >= Ogre::Math::TWO_PI) { t -= Ogre::Math::TWO_PI; };

			Ogre::SceneNode* node = nodes.at(i).first;
			Ogre::Vector3 newPosition(
				xc + ma * cos(t) * cos(fi) - mi * sin(t) * sin(fi),
				OgreAddOns::getDerivedPosition(node).y,
				zc + ma * cos(t) * sin(fi) + mi * sin(t) * cos(fi)
				);
			OgreAddOns::setDerivedPosition(node, &newPosition);

			if (rot == rot) OgreAddOns::setOrientationYXZ(node, rot - t * 180./Ogre::Math::PI, 0);
		}

		return false;
	}

}