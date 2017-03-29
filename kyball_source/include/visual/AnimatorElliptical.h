///////////
// Animator Elliptical
///////////

#pragma once
#ifndef AnimatorElliptical_H
#define AnimatorElliptical_H

#include "visual\Visual.h"
#include <vector>
#include <utility>

namespace P3D {

	class AnimatorElliptical : public Visual {

	protected:

		std::vector<std::pair<Ogre::SceneNode*, double> > nodes; /// 't' paramater for every node

		double xc;	/// } Ellipse 2D Center
		double zc;	/// }
		double ma;	/// major
		double mi;	/// minor
		double fi;	/// angle (ellipse's)

		double rot; /// rotate children according to position in ellipse + this angle
		double sp;

	public:

		AnimatorElliptical(Ogre::SceneNode*, double major, double minor, double angle, double rotation, double speed, double start);

		virtual bool run(double);
	};
}

#endif