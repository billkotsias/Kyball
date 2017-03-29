#include "StdAfx.h"
#include "visual\AnimationStatesList.h"

#include "visual\VisualBoss.h"
#include "visual\AnimationStateWrapper.h"

namespace P3D {

	AnimationStatesList::AnimationStatesList(std::vector<AnimationStateWrapper*> _anims, AnimationStatesList::LoopType _loop, double _startPos) : loopType(_loop), currentAnim(0)
	{
		anims = _anims;
		anims.at(0)->setEnabled(true);

		if (_startPos != _startPos) _startPos = getTotalLength() * rand() / RAND_MAX; /// randomize
		if (_startPos) run(_startPos);

		VisualBoss::getSingletonPtr()->insert(this);
	}

	AnimationStatesList::~AnimationStatesList()
	{
		for (int i = anims.size() - 1; i >= 0; --i) {
			delete anims.at(i);
		}
		anims.clear();
	}

	bool AnimationStatesList::run(double time)
	{
		while (currentAnim >= 0 && time > 0)
		{
			time = anims.at( currentAnim )->run(time);
			if (time >= 0) advanceCurrentAnim();
		}
		return false;
	}

	double AnimationStatesList::getTotalLength() const
	{
		double sum = 0;
		for (int i = anims.size() - 1; i >= 0; --i) {
			sum += anims.at(i)->getLength();
		}
		return sum;
	}

	void AnimationStatesList::advanceCurrentAnim()
	{
		AnimationStateWrapper* asw;

		switch (loopType)
		{
			case AnimationStatesList::none:
				if ( currentAnim < (int)(anims.size()-1) ) {
					anims.at(currentAnim)->setEnabled(false); /// if not last in line, disable current
				}
				if ( ++currentAnim >= (int)anims.size() ) {
					currentAnim = -1;
				} else {
					asw = anims.at(currentAnim);
					asw->setEnabled(true);
					asw->reset();
				}
				break;

			case AnimationStatesList::loop:
				anims.at(currentAnim)->setEnabled(false);

				if ( ++currentAnim >= (int)anims.size() ) {
					currentAnim = 0;
				}
				asw = anims.at(currentAnim);
				asw->setEnabled(true);
				asw->reset();
				break;

			case AnimationStatesList::random:
				anims.at(currentAnim)->setEnabled(false);

				currentAnim = anims.size() * rand() / (RAND_MAX+1);
				asw = anims.at(currentAnim);
				asw->setEnabled(true);
				asw->reset();
				break;
		}
	}

//#ifdef _DEBUG
//	/// debug
//	std::string AnimationStatesList::getNames() {
//		std::string name = "";
//		for (unsigned int i = 0; i < anims.size(); ++i) name += anims.at(i)->getName();
//		return name;
//	}
//#endif

}