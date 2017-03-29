#include "StdAfx.h"
#include "game\ai\BounceAI.h"
#include "game\action\ReflectAction.h"

#include <math\Math.h>

namespace P3D {

	BounceAI::BounceAI(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position, bool mover) : AIPosVel(obj, parent, position, mover) {
		mass = Infinite;
	}

	Action* BounceAI::getAction() {
		static ReflectAction reflectAction(Action::HIGH); /// all objects are reflected on contact with a "container" object
		return &reflectAction;
	}

	void BounceAI::receiveAction(P3D::Action *action) {
		/// <TODO> : make a "visual effect" at the point of contact with another object
	}
}