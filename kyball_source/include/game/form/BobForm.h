///////////
// BobForm
///////////

#pragma once
#ifndef BobForm_H
#define BobForm_H

#include "BallForm.h"

namespace P3D {

	class BobForm : public BallForm {

	public:

		static const int BOBS_MAX_FRAME = 1; /// number of flip-book images for every bob

		BobForm(GameplayBase*, Ogre::BillboardSet*, BallState::BallType ballType, double radius = BallState::BALL_RADIUS);
		virtual ~BobForm() {};

		virtual void update();
	};
}

#endif