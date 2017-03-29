#include "StdAfx.h"
#include "game\action\Action.h"

namespace P3D {

	Action::Action(Action::Type typ) : collisionParams(CollisionParams::NO_COLLISION) {
		type = typ;
	}

	Action::~Action() {
	}

	Action::Type Action::getType() const {
		return type;
	}

	Action* Action::copy() {
		return new Action(type);
	}

	Action* Action::embedCollision(P3D::CollisionParams params) {
		collisionParams = params;
		return this;
	}

}