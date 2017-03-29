#include <stdafx.h>
#include "visual\CallEvery.h"
#include "visual\VisualBoss.h"

namespace P3D {

	bool ICallEvery::run(double time)
	{
		timeToCall -= time;
		while (timeToCall <= 0)
		{
			timeToCall += repeatEvery;

			if (timesToCall > 0 && --timesToCall == 0) {
				callFunction();
				return true;
			}

			callFunction();
		}

		return false;
	}
}