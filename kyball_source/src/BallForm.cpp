#include "StdAfx.h"
#include "game\form\BallForm.h"

#include "P3DApp.h"
#include "SceneBaseCreator.h"
#include "game\GameplayBase.h"
#include "game\form\FormState.h"
#include "game\form\BallState.h"
#include "collision\Sphere.h"

#include "OgreAddOns.h"

#include <OgreBillboardSet.h>
#include <OgreBillboard.h>
#include <OgreColourValue.h>

namespace P3D {

	const int BallForm::BBB_MAX_FRAME = 1;
	const double BallForm::FLIP_SPEED = 2;

	BallForm::BallForm(GameplayBase* base, Ogre::BillboardSet* bbset, BallState::BallType ballType, bool countForScore, double radius) :
		ShadowCaster(base->getShadowManager(), ShadowCaster::BLOB), billboard(0), parent(0), frameNumber(0) {

		/// 1st - create <CollisionObject> and attach to 'pivot'
		collisionObject = new Sphere(pivot, 0); /// 'Sphere' radius will be set right below

		/// 2nd - create <billboard> and its container
		changeBBSet(bbset);

		/// set the default "depiction", but this can change by anyone, anytime
		setState(new BallState(ballType, countForScore, radius));
	}

	BallForm::~BallForm() {

		/// destroy <billboard>
		changeBBSet(0);

		/// <CollisionObject> is destroyed by parent ~Form() !

		/// destroy our copy of 'state'
		Form::setState(0); /// use ancestor function because it's overriden in this class and won't do 'delete'
	}

	void BallForm::changeBBSet(Ogre::BillboardSet* bbset) {

		/// destroy previous billboard
		if (billboard) {
			billboard->mParentSet->removeBillboard(billboard);
			billboard = 0;
			parent = 0;
		}

		/// create new billboard
		if (bbset) {
			parent = bbset->getParentSceneNode();
			billboard = bbset->createBillboard(0, 0, 0);
			resetRadius();
		}
	}

	bool BallForm::updateShadow(Ogre::Vector2* position, Ogre::Vector2* scaling) {
		const Ogre::Vector3& pos = billboard->getPosition();
		position->x = pos.x;
		position->y = pos.z;
		if (formState && formState->getState() == FormState::BALL) {
			*scaling = ((BallState*)formState)->getBallRadius() * 2.1;
		} else {
			*scaling = 1;
		}
		return false;
	}

	void BallForm::setState(FormState* newState) {
		if (newState && newState->getState() == FormState::BALL) {
			/// received a <BallState> message which I can interpret!
			BallState* ballState = (BallState*)newState;

			/// - colour
			if (ballState->getBallType() == BallState::UNCHANGED)
				ballState->setBallType(((BallState*)formState)->getBallType()); /// keep previous colour!

			/// - replace old state
			delete formState;
			formState = ballState;

			resetRadius(); /// <!>
		}
	}

	void BallForm::resetRadius() {
		/// - billboard
		double radius = ((BallState*)formState)->getBallRadius();
		Ogre::BillboardSet* bbset = billboard->mParentSet;
		billboard->setDimensions(radius * bbset->getDefaultWidth() / BallState::BALL_RADIUS, radius * bbset->getDefaultHeight() / BallState::BALL_RADIUS);

		/// - CollisionObject
		((Sphere*)collisionObject)->setLocalRadius(radius);
	}

	void BallForm::update() {

		Ogre::Vector3 worldPos = OgreAddOns::getDerivedPosition(pivot);
		if (parent) billboard->setPosition( OgreAddOns::worldToLocalParent(parent, &worldPos) );

		/// advance animation
		frameNumber += (FLIP_SPEED / P3DApp::CYCLES_PER_SEC);
		while (frameNumber >= BBB_MAX_FRAME) frameNumber -= BBB_MAX_FRAME;

		/// set appropriate image
		int bbRow = ((BallState*)formState)->getBallType();
		if (bbRow >= BallState::BOB) bbRow -= 8; /// reset rows after last colored ball because of different texture used <!!!>
		billboard->setTexcoordIndex((int)frameNumber + BBB_MAX_FRAME * bbRow);
	}

}
