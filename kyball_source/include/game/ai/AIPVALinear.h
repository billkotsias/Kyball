// ------------.................................
// AIPVALinear - Linear animation
// ------------`````````````````````````````````

#pragma once
#ifndef AIPVALinear_H
#define AIPVALinear_H

#include "AIPVAnime.h"

#include <OgreVector3.h>

namespace P3D {

	class AIPVALinear : public AIPVAnime {

	private:

		int cycles;
		Ogre::Vector3 final;

	public:

		// => object
		//	  final position to reach
		//	  duration, in seconds
		AIPVALinear(AIPosVel*, const Ogre::Vector3&, double);

		virtual bool run(); /// run a cycle
	};

}

#endif