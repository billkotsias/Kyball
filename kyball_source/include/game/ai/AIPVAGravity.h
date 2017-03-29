// ------------.................................
// AIPVAGravityAcc - Gravity with acceleration
// ------------`````````````````````````````````

#pragma once
#ifndef AIPVAGravity_H
#define AIPVAGravity_H

#include "AIPVAnime.h"

namespace P3D {

	class AIPVAGravity : public AIPVAnime {

	private:

		double gravity;

	public:

		// => object
		//	  gravity in "units / second"
		AIPVAGravity(AIPosVel*, double);

		virtual bool run(); /// run a cycle
	};

}

#endif