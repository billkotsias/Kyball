#include "StdAfx.h"
#include "visual\Billboard.h"

#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <game\GameplayCollection.h>

namespace P3D {

	Billboard::Billboard(GameplayCollection* _group, double _ttl, double start, double end, double current, double step) : frameStart(start), frameEnd(end),
		frameNumber(current), frameStep(step), ttl(_ttl), group(_group) {

		group->storeVisual(this); /// I'm an "orphan"; I have to be in a collection, but I'll die before it does... THAT causes trouble.
	};

	bool Billboard::run(double time) {

		ttl -= time;
		if (ttl <= 0) {
			/// <die> right away
			group->visuals.remove(this);
			return true;
		}

		frameNumber += frameStep * time;
		while (floor(frameNumber) > floor(frameEnd)) {
			frameNumber += frameStart - frameEnd;
		}

		billboard->setTexcoordIndex(floor(frameNumber));

		return false;
	}

	Billboard::~Billboard() {
		billboard->mParentSet->removeBillboard(billboard);
	}
}