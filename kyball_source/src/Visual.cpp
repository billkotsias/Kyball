#include "StdAfx.h"
#include <visual\Visual.h>

#include <visual\VisualBoss.h>

namespace P3D {

	Visual::~Visual() {
		VisualBoss::getSingletonPtr()->remove(this);
	}
}
