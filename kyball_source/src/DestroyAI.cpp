#include "StdAfx.h"
#include "game\ai\DestroyAI.h"
#include "game\action\DestroyAction.h"

#include <math\Math.h>

namespace P3D {

	DestroyAI::DestroyAI(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position, bool mover) : AIPosVel(obj, parent, position, mover) {
		mass = Infinite;
	}

	Action* DestroyAI::getAction() {
		static DestroyAction destroyAction(Action::HIGH);
		return &destroyAction;
	}
}