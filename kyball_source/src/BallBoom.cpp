#include "StdAfx.h"
#include "visual\BallBoom.h"

#include "game\GameplayBase.h"

#include "tween\TLinear.h"
#include <OgreColourValue.h>

namespace P3D {

	const std::vector<Ogre::ColourValue> BallBoom::ballColours = BallState::initBallColours(1, 0.8, 0.5, 0.45);

	BallBoom::BallBoom(GameplayBase* base, BallState::BallType ballType, Ogre::Vector3 pos, double _ttl) : Billboard(base->getCollection(), _ttl) {

		Ogre::BillboardSet* bbset = base->getBBSet(GameplayBase::BALL_BOOM);
		billboard = bbset->createBillboard(pos);
		billboard->setTexcoordIndex(floor(frameNumber));
		Ogre::ColourValue startCol = ballColours.at(ballType);
		Ogre::ColourValue endCol = startCol;
		endCol.a = 0;
		billboard->setColour(startCol);

		/// WHO TELLS ME THAT <TPlayer> is UPDATED ALONGSIDE <VisualBoss>? Ah? NOONE! But this way, I don't re-write the same code... HA!
		/// IN FACT, all these ...->run(time) classes should be inhereted by one, and they should all be updated at once, that's the RIGHT solution!
		registerTween( new TLinear<Ogre::Billboard, Ogre::ColourValue>(
			billboard, endCol, ttl, &Ogre::Billboard::getColour, &Ogre::Billboard::setColour)
		);

		//registerTween( new TLinear<Ogre::Billboard, Ogre::Radian>(
		//	billboard, Ogre::Degree(360), ttl, &Ogre::Billboard::getRotation, &Ogre::Billboard::setRotation)
		//);
	};

	BallBoom::~BallBoom() {
		deleteTweens();
	}

}