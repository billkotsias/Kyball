#include "StdAfx.h"
#include "ShadowCaster.h"

#include "ShadowManager.h"

namespace P3D {

	ShadowCaster::ShadowCaster(ShadowManager* man, ShadowCaster::Type type) : shadowManager(man) {
		if (shadowManager) shadowManager->registerCaster(this, type);
	}

	ShadowCaster::~ShadowCaster() {
		if (shadowManager) shadowManager->unregisterCaster(this);
	}

}