#include "StdAfx.h"
#include "game\form\BallState.h"

namespace P3D {

	const double BallState::BALL_RADIUS = 0.5;

	const BallState::BallType BallState::ballTypes[BallState::MAX_TYPES + BallState::MAX_BOBS] = {
		BallState::RED,BallState::GREEN,BallState::BLUE,BallState::CYAN,
		BallState::MAGENTA,BallState::YELLOW,BallState::WHITE,BallState::BLACK,
		BallState::BOB, BallState::BRO
	};

	const std::vector<Ogre::ColourValue> BallState::initBallColours(double c0, double c1, double c2, double c3) {
		std::vector<Ogre::ColourValue> colours;
		colours.resize(BallState::MAX_TYPES + BallState::MAX_BOBS);

		colours[BallState::RED] = Ogre::ColourValue(c0,c1,c1);
		colours[BallState::GREEN] = Ogre::ColourValue(c1,c0,c1);
		colours[BallState::BLUE] = Ogre::ColourValue(c1,c1,c0);
		colours[BallState::CYAN] = Ogre::ColourValue(c2,c0,c0);
		colours[BallState::MAGENTA] = Ogre::ColourValue(c0,c2,c0);
		colours[BallState::YELLOW] = Ogre::ColourValue(c0,c0,c2);
		colours[BallState::WHITE] = Ogre::ColourValue(c0,c0,c0);
		colours[BallState::BLACK] = Ogre::ColourValue(c3,c3,c3);

		///was:colours[BallState::BOB] = Ogre::ColourValue(c0,c0,c0);

		return colours;
	}

}