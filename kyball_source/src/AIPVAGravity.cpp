#include "StdAfx.h"
#include "game\ai\AIPVAGravity.h"

#include "game\ai\AIPosVel.h"
#include <OgreVector3.h>
#include "P3DApp.h"

namespace P3D {

	AIPVAGravity::AIPVAGravity(AIPosVel* inObj, double grav) : AIPVAnime(inObj) {
		gravity = grav * (1. / (P3DApp::CYCLES_PER_SEC * P3DApp::CYCLES_PER_SEC));
	}

	bool AIPVAGravity::run() {

		Ogre::Vector3 newVel = obj->getVelocity();
		newVel.y += gravity;
		obj->setVelocity(newVel);

		return false; /// never "finishes"; must be deleted by the caller
	}
}