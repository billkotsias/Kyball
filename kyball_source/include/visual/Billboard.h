////////////
// Billboard - visual element
////////////

#pragma once
#ifndef Billboard_H
#define Billboard_H

#include "visual\Visual.h"
#include <math\Math.h>

namespace Ogre {
	class Billboard;
}

namespace P3D {

	class GameplayCollection; /// for clean clean-up!

	class Billboard : public Visual {

	protected:

		GameplayCollection* group; /// for clean clean-up!

		Ogre::Billboard* billboard;	/// billboard depiction
		double frameStart;			/// <inclusive>
		double frameEnd;			/// <inclusive>
		double frameNumber;			/// current animation frame
		double frameStep;
		double ttl;

	public:

		// constructor
		// => time to live
		//	  start frame
		//	  end frame
		//	  current frame
		//	  frame step
		Billboard(GameplayCollection* _group, double _ttl = Infinite, double start = 0, double end = 0, double current = 0, double step = 0);
		virtual ~Billboard();

		// overrides
		virtual bool run(double);

	};
}

#endif