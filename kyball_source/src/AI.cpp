#include "StdAfx.h"
#include "game\ai\AI.h"

#include "collision\CollisionObject.h"
#include "game\form\Form.h"
#include "game\action\NoAction.h"

namespace P3D {

	AI::AI(GameplayObject* obj) : form(0), mass(1), owner(obj), linked(false) {
	}

	AI::~AI() {
		// was: attachForm(0); /// detach any attached 'Form'
		// NOTE : on destruction, any derived classes must detach their Forms themselves; see ~BallAI()
	}

	CollisionObject* AI::getCollisionObject() {
		if (form) return form->getCollisionObject();
		return 0;
	}

	Action* AI::getAction() {
		static NoAction noAction;
		return &noAction;
	}

	bool AI::attachForm(P3D::Form *inForm) {

		/// remove any previously attached 'Form'
		if (form) {
			/// - detach <pivot>
			Ogre::SceneNode* pivot = form->getPivot();
			if (pivot->getParent()) pivot->getParent()->removeChild(pivot);
			/// - detach <velocity>
			form->getCollisionObject()->setVelocity(0);
			form->getCollisionObject()->checkFutureCollision = false; /// ... just in case
		}

		/// ... the new "inForm" will have to be attached by derived classes...
		return false;
	}
}