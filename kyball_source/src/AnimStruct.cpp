#include "StdAfx.h"
#include "AnimStruct.h"

#include <fsl\FSLArray.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>

#include <OgreEntity.h>

namespace P3D {

	// Anima

	void Anima::checkEnable() {
		static const double tolerance = 0.001;

		if (pairs.empty() || (currValue < tolerance && pairs.front().step == 0)) { /// <0.001> = tolerance for accumulative errors due to "stepping"
			animation->setEnabled(false);
		} else {
			animation->setEnabled(true);
		}
	}

	bool Anima::run(double remainingTime) {
		static const double tolerance = 0.00001;

		while (remainingTime > tolerance) { /// tolerance due to precision errors
			TSPair& pair = pairs.front();
			double runTime = (remainingTime <= pair.time) ? remainingTime : pair.time;
			currValue += runTime * pair.step;
			animation->setTimePosition(currValue);

			pair.time -= runTime;
			if (pair.time <= tolerance) {
				pairs.pop_front();
				checkEnable();
				if (pairs.empty()) return true;
			}

			remainingTime -= runTime;
		}
		return false;
	}

	// AnimaSet

	AnimaSet::AnimaSet(Ogre::Entity* entity, FANLib::FSLClass *fslAnimaSet) {

		/// AnimaSet "setID" is ignored here, but should be taken into account by the caller

		/// build animas
		FANLib::FSLArray* fslAnimas = fslAnimaSet->getArray("animas");
		animas.resize(fslAnimas->getSize());
		for (unsigned int i = 0; i < fslAnimas->getSize(); ++i) {

			/// build anima
			Anima& anima = animas.at(i);
			FANLib::FSLClass* fslAnima = fslAnimas->getClass(i);
			anima.animation = entity->getAnimationState(fslAnima->getCString("name"));
			anima.currValue = fslAnima->getReal("start");

			/// build anima's pairs
			FANLib::FSLArray* fslPairs = fslAnima->getArray("pairs");
			anima.pairs.resize(fslAnimas->getSize());
			double oldValue = anima.currValue; /// used to calc step
			for (unsigned int j = 0; j < fslPairs->getSize(); ++j) {

				/// build pair
				TSPair& pair = anima.pairs.at(j);
				FANLib::FSLClass* fslPair = fslPairs->getClass(j);
				pair.time = fslPair->getReal("t");
				double newValue = fslPair->getReal("v");
				pair.step = (newValue - oldValue) / pair.time;
				oldValue = newValue;
			}
		}
	}

	AnimaSet AnimaSet::start() {
		for (int i = animas.size() - 1; i >= 0; --i) {
			animas.at(i).checkEnable();
		}
		return *this;
	}

	bool AnimaSet::run(double time) {

		for (int i = animas.size() - 1; i >= 0; --i) {
			if (animas.at(i).run(time)) {
				animas.erase(animas.begin() + i);
			}
		}

		return animas.empty();
	}
}