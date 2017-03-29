#pragma once
#ifndef AnimationStatesList_H
#define AnimationStatesList_H

#include "visual\Visual.h"
#include <vector>

namespace P3D {

	class AnimationStateWrapper;

	class AnimationStatesList : public Visual {

	public:

		enum LoopType {
			none,
			loop,
			random,
			combined,
		};

	protected:

		std::vector<AnimationStateWrapper*> anims;
		int currentAnim;
		LoopType loopType;

		void advanceCurrentAnim();

	public:

		AnimationStatesList(std::vector<AnimationStateWrapper*> _anims, AnimationStatesList::LoopType _loop, double _startPos);
		virtual ~AnimationStatesList();

		virtual bool run(double);

		double getTotalLength() const;

//#ifdef _DEBUG
//		/// debug
//		std::string getNames();
//#endif
	};
}

#endif