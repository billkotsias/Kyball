#include "StdAfx.h"
#include "visual\AnimationStatesCombined.h"

#include "visual\VisualBoss.h"
#include "visual\AnimationStateWrapper.h"

namespace P3D {

	AnimationStatesCombined::AnimationStatesCombined(std::vector<AnimationStateWrapper*> _anims) {
		anims = _anims;
		for (unsigned int i = 0; i < anims.size(); ++i) {
			anims.at(i)->setEnabled(true);
			anims.at(i)->setLoop(true);
		}
		VisualBoss::getSingletonPtr()->insert(this);
	}

	AnimationStatesCombined::~AnimationStatesCombined() {
		for (unsigned int i = 0; i < anims.size(); ++i) {
			delete anims.at(i);
		}
		anims.clear();
	}

	bool AnimationStatesCombined::run(double time) {
		for (unsigned int i = 0; i < anims.size(); ++i) {
			anims.at(i)->run(time);
		}
		return false;
	}
}