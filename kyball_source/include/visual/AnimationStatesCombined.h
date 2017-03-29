#pragma once
#ifndef AnimationStatesCombined_H
#define AnimationStatesCombined_H

#include "visual\Visual.h"
#include <vector>

namespace P3D {

	class AnimationStateWrapper;

	class AnimationStatesCombined : public Visual {

	protected:

		std::vector<AnimationStateWrapper*> anims;

	public:

		AnimationStatesCombined(std::vector<AnimationStateWrapper*> _anims);

		virtual ~AnimationStatesCombined();

		virtual bool run(double);
	};
}

#endif