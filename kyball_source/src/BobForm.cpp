#include "StdAfx.h"
#include "game\form\BobForm.h"

#include "P3DApp.h"
#include "OgreAddOns.h"

namespace P3D {

	BobForm::BobForm(GameplayBase* base, Ogre::BillboardSet* bbset, BallState::BallType ballType, double radius)
		: BallForm(base, bbset, ballType, true, radius) { /// always "count for score" <!>
	}

	void BobForm::update() {

		Ogre::Vector3 worldPos = OgreAddOns::getDerivedPosition(pivot);
		if (parent) billboard->setPosition( OgreAddOns::worldToLocalParent(parent, &worldPos) );

		/// advance animation
		frameNumber += (FLIP_SPEED / P3DApp::CYCLES_PER_SEC);
		while (frameNumber >= BOBS_MAX_FRAME) frameNumber -= BOBS_MAX_FRAME;

		/// set appropriate image
		int bbRow = ((BallState*)formState)->getBallType() - 8; /// <!!!>
		billboard->setTexcoordIndex((int)frameNumber + BOBS_MAX_FRAME * bbRow);
	}

}
