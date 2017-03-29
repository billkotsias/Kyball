///////////
// BallForm - a "flip-book" animated billboard
///////////

#pragma once
#ifndef BallForm_H
#define BallForm_H

#include "ShadowCaster.h"
#include "game\form\Form.h"
#include "game\form\BallState.h"

namespace Ogre {
	class BillboardSet;
	class Billboard;
	class SceneNode;
}

namespace P3D {

	class GameplayBase;

	class BallForm : public Form, public ShadowCaster {

	private:

		void resetRadius();

	protected:

		Ogre::Billboard* billboard;	/// billboard depiction
		Ogre::SceneNode* parent;	/// this billboard's BillboardSet parent SceneNode
		double frameNumber;			/// current animation frame

	public:

		static const int BBB_MAX_FRAME;	/// number of flip-book images for every ball (should be 16)
		static const double FLIP_SPEED;	/// images flipped/sec

		BallForm(GameplayBase*, Ogre::BillboardSet*, BallState::BallType, bool countForScore = false, double = BallState::BALL_RADIUS);
		virtual ~BallForm();

		void changeBBSet(Ogre::BillboardSet*);

		/// overrides
		virtual void update();
		virtual void setState(FormState*);	/// message that alters 'Form'; <NOTE> : also alters <CollisionObject> !

		virtual bool updateShadow(Ogre::Vector2*, Ogre::Vector2*);

		Ogre::Billboard* getBillboard() { return billboard; };
	};
}

#endif