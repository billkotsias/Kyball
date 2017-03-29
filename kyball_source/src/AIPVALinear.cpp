#include "StdAfx.h"
#include "game\ai\AIPVALinear.h"

#include "game\ai\AIPosVel.h"
#include "P3DApp.h"

namespace P3D {

	AIPVALinear::AIPVALinear(P3D::AIPosVel *inObj, const Ogre::Vector3 &fPos, double dur) : AIPVAnime(inObj), final(fPos) {
		cycles = (double)P3DApp::CYCLES_PER_SEC * dur;
	}

	// was:
	//bool AIPVALinear::run() {
	//	obj->setPosition( obj->getPosition() + (final - obj->getPosition()) / cycles);
	//	if (--cycles) return false;
	//	return true; /* finished */;
	//}

	bool AIPVALinear::run() {

		if (--cycles) {
			obj->setVelocity( (final - obj->getPosition()) / cycles);
			return false;
		} else {
			obj->setVelocity(Ogre::Vector3::ZERO);
			obj->setPosition(final);
			return true;
		}
	}

}