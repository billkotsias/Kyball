#include "StdAfx.h"
#include "game\animal\Animal.h"

#include <fsl\FSLArray.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>
#include "P3DScripts.h"
#include "sound\Sound.h"

#include "SceneBaseCreator.h"
#include <OgreEntity.h>
#include "tween\TLinear.h"
#include "tween\TSin.h"
#include "tween\TCos.h"
#include "gui\HintBox.h"

#include "BurstBoss.h"
#include "OgreTagPoint.h"

namespace P3D {

	// AnimalAnim

	double Animal::fadeSpeed;
	double Animal::fadeTime;

	AnimalAnim::AnimalAnim() : speed(1), weightSpeed(0), _volatile(false), pauseAtEnd(false), repeatable(true), sound(0), soundHandle(0), particleSystem(0), currentDelay(-1) {
	}

	void AnimalAnim::play(const Ogre::Vector3& pos) {
		for (unsigned int i = 0; i < animStates.size(); ++i) {
			animStates[i]->setEnabled(true);
		}
		/// play sound
		if (sound && sound->buffer) {
			soundHandle = sound->play(pos);
		}
		/// prepare for burst?
		if (particleSystem) {
			currentDelay = particleDelay;
		}
	}

	void AnimalAnim::setSound(const std::string& name) {
		sound = Sound::getScripted(name.c_str());
		sound->init();
	}

	void AnimalAnim::reset() {
		for (unsigned int i = 0; i < animStates.size(); ++i) {
			Ogre::AnimationState* animState = animStates[i];
			animState->setEnabled(false);
			animState->setTimePosition(0);
			animState->setWeight(1);
			animState->setLoop(false);
		}
		weightSpeed = 0;

		//if ( sound && (*(sound->buffer)) && soundHandle ) {
		if ( soundHandle ) {
			//std::cout << "AnimalAnim::reset\n";
			Sound::stop(soundHandle);
			soundHandle = 0;
		}
	}

	double AnimalAnim::remainingRuntime() {
		/// the longest sub-animation time is returned
		double remainingRuntime = 0;
		for (unsigned int i = 0; i < animStates.size(); ++i) {
			double subAnimTime = ( animStates[i]->getLength() - animStates[i]->getTimePosition() ) / speed;
			if (remainingRuntime < subAnimTime) remainingRuntime = subAnimTime;
		}
		return remainingRuntime;
	}

	double AnimalAnim::run(double time) {

		/// burst?
		if (currentDelay >= 0)
		{
			currentDelay -= time;
			if (currentDelay <= 0)
			{
				burst->newBurst( particleSystem, entity, particleBone );
			}
		}

		/// is this the hack where we are "fading-in" the "rest" skeletal pose?
		if (weightSpeed > 0) {
			double weight = animStates[0]->getWeight() + weightSpeed * time;
			if (weight > 1.) weight = 1.;
			for (unsigned int i = 0; i < animStates.size(); ++i) {
				animStates[i]->setWeight(weight);
			}
			if (weight == 1.) return 1; /// fade-in has finished
			return 0;
		}

		/// run all animation states
		/// NOTE : sub-animations are assumed to have the same length : only the first one's properties are read!
		double remainingRuntime = this->remainingRuntime();
		double runtime = (remainingRuntime < time) ? remainingRuntime : time;
		for (unsigned int i = 0; i < animStates.size(); ++i) {
			animStates[i]->addTime(runtime * speed);
		}

		/// is animation being "faded-out"?
		if (weightSpeed < 0) {
			double remainingWeightTime = animStates[0]->getWeight() / (-weightSpeed);
			double weightRuntime = (remainingWeightTime < time) ? remainingWeightTime : time;
			for (unsigned int i = 0; i < animStates.size(); ++i) {
				animStates[i]->setWeight(animStates[i]->getWeight() + weightRuntime * weightSpeed);
			}

			if (weightRuntime < runtime || pauseAtEnd) runtime = weightRuntime; /// if 'pauseAtEnd = true' don't count skeletal runtime as its meaningless
			return time - runtime; /// <NOTE> : return "finished" even if 'pauseAtEnd = true'! This is another way to <unlock> this animation (besides "reset")
		}

		if (pauseAtEnd) return 0;	/// don't report "finished"
		return time - runtime;		/// if > 0, this animation has finished
	}

	// Animal

	std::vector<Animal::Animation> Animal::enumToAnim;
	std::map<Animal::Animation, std::string> Animal::animToString;
	const char* const Animal::ANIM_MORPH_POSTFIX = "M";

	Animal::Animal(Ogre::Entity* entity, const std::string& cosmosID, BurstBoss* burst) : isSmiling(false), isAnxious(false)
	{
		animalNode = entity->getParentSceneNode();
		/// get rest from node
		Ogre::SceneNode* talkNode = static_cast<Ogre::SceneNode*>(animalNode->getChild(0));
		talkScale = talkNode->getScale().x; /// uniform
		talkNode->setScale(0,0,0);
		talkColourPass = static_cast<Ogre::Entity*>(talkNode->getAttachedObject(0))->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
		talkColour = talkColourPass->getSelfIllumination();

		buildOnce();

		/// get all available animation states from entity
		anims.resize(Animal::_last);
		for (std::map<Animal::Animation, std::string>::iterator it = animToString.begin(); it != animToString.end(); ++it) {
			try {
				anims[it->first].setSound(cosmosID + "\\" + it->second);
				anims[it->first].animStates.push_back(entity->getAnimationState(it->second));
			} catch(...) {}
			try {
				anims[it->first].animStates.push_back(entity->getAnimationState(it->second + ANIM_MORPH_POSTFIX));
			} catch(...) {}
		}

		/// get custom data
		FANLib::FSLArray* fslASpairs = P3DScripts::p3dAnimes->getRoot()->getArray(cosmosID.c_str());
		for (unsigned int i = 0; i < fslASpairs->getSize(); ++i) {
			FANLib::FSLClass* fslASpair = fslASpairs->getClass(i);
			P3D::AnimalAnim& anim = anims[enumToAnim[fslASpair->getInt("a")]];
			anim.speed = fslASpair->getReal("s") / 100; /// custom speed
			std::string particleStr = fslASpair->getCString("particles");
			if ( particleStr.size() ) {
				anim.particleSystem = burst->createTemplate( particleStr );
				anim.particleBone = fslASpair->getCString("partBone");
				anim.particleDelay = fslASpair->getReal("partDelay");
				anim.entity = entity;
				anim.burst = burst;
			}
		}

		/// "group" available <basic> loops; also set as <volatile>
		FANLib::FSLArray* fslBasicLoops = P3DScripts::p3dAnimes->getRoot()->getArray("basicLoops");
		double totalSum = 0;
		for (unsigned int i = 0; i < fslBasicLoops->getSize(); ++i) {
			FANLib::FSLClass* fslBLData = fslBasicLoops->getClass(i);

			AnimalAnim* anim = &anims[enumToAnim[fslBLData->getInt("a")]];
			if (!anim->animStates.size()) continue; /// no respective animation found in entity!

			/// - set as <volatile> by default
			anim->_volatile = true;

			/// - read and set probability
			double probability = fslBLData->getReal("p");
			totalSum += probability;

			/// - read and set "repeatability"
			anim->repeatable = (fslBLData->getInt("r") == 0) ? false : true;

			std::pair<std::map<AnimalAnim*, double>::iterator, bool> errorCheck;
			errorCheck = basicLoops.insert( std::pair<AnimalAnim*, double>(anim, probability) );
			if ( !errorCheck.second ) {
				throw "Animation probability doubly defined";
			}
		}
		/// - convert probability to "limits"
		double sum = 0;
		for (std::map<AnimalAnim*, double>::iterator it = basicLoops.begin(); it != basicLoops.end(); ++it) {
			sum += it->second;
			it->second = sum / totalSum; /// "limits" are "increased" as the iterator is increased (exploited by 'chooseBasic()' )
		}

		/// set "pauseAtEnd" (this could go into the script)
		anims[Animal::win].pauseAtEnd = true;
		anims[Animal::lose].pauseAtEnd = true;
		anims[Animal::smiling].pauseAtEnd = true;
		anims[Animal::anxious].pauseAtEnd = true;

		/// other <volatile> animations
		FANLib::FSLArray* fslVola = P3DScripts::p3dAnimes->getRoot()->getArray("volatile");
		for (unsigned int i = 0; i < fslVola->getSize(); ++i) {
			anims[enumToAnim[fslVola->getInt(i)]]._volatile = true;
		}
			
	}

	Animal::~Animal() {
		if (currAnimation) currAnimation->reset();
	}

	void Animal::buildOnce() {
		static bool built = false;
		if (built) return;
		built = true;

		/// <P3DAnimations.fsl> must have been parsed by this point

		/// - enum -> anim
		FANLib::FSLEnum* enu = P3DScripts::p3dAnimes->getEnum("Animation");
		enumToAnim.resize(enu->getSize(), Animal::rest);
		enumToAnim[enu->getValue("rest")] = Animal::rest;
		enumToAnim[enu->getValue("basic")] = Animal::basic;
		enumToAnim[enu->getValue("tick1")] = Animal::tick1;
		enumToAnim[enu->getValue("tick2")] = Animal::tick2;
		enumToAnim[enu->getValue("tick3")] = Animal::tick3;
		enumToAnim[enu->getValue("look")] = Animal::look;
		enumToAnim[enu->getValue("happy1")] = Animal::happy1;
		enumToAnim[enu->getValue("happy2")] = Animal::happy2;
		enumToAnim[enu->getValue("happy3")] = Animal::happy3;
		enumToAnim[enu->getValue("sad")] = Animal::sad;
		enumToAnim[enu->getValue("win")] = Animal::win;
		enumToAnim[enu->getValue("lose")] = Animal::lose;
		enumToAnim[enu->getValue("smiling")] = Animal::smiling;
		enumToAnim[enu->getValue("anxious")] = Animal::anxious;

		/// - anim -> string
		FANLib::FSLArray* enumToStr = P3DScripts::p3dAnimes->getRoot()->getArray("enumToString");
		for (unsigned int i = 0; i < enumToStr->getSize(); ++i) {
			FANLib::FSLClass* ANpair = enumToStr->getClass(i);
			animToString[enumToAnim[ANpair->getInt("a")]] = ANpair->getCString("n");
		}

		/// - animation fadeout speed
		Animal::fadeSpeed = P3DScripts::p3dAnimes->getRoot()->getReal("fadeSpeed");
		Animal::fadeTime = 1 / Animal::fadeSpeed;
	}

	void Animal::start() {

		/// reset all
		for (unsigned int i = 0; i < anims.size(); ++i) {
			if (anims[i].animStates.size()) anims[i].reset();
		}

		nextAnimation = _none;
		idle = false;
		isSmiling = false;
		isAnxious = false;
		facialPlaying.clear();

		currAnimation = &anims[basic]; /// was : chooseBasic();
		currAnimation->play(animalNode->_getDerivedPosition());
	}

	void Animal::talk(bool enable)
	{
		idle = enable;

		deleteTweens();
		Ogre::SceneNode* talkNode = static_cast<Ogre::SceneNode*>(animalNode->getChild(0));
		double time = HintBox::APPEAR_TIME + HintBox::UNFOLD_TIME;
		if (enable)
		{
			talkColourPass->setSelfIllumination( Ogre::ColourValue::ZERO );
			registerTween(new TCos<Ogre::Pass, Ogre::ColourValue>(
				talkColourPass, talkColour, time, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
			talkNode->setScale( Ogre::Vector3::ZERO );
			registerTween(new TSin<Ogre::SceneNode, Ogre::Vector3>(
				talkNode, Ogre::Vector3(talkScale), time, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		} else {
			talkColourPass->setSelfIllumination( talkColour );
			registerTween(new TSin<Ogre::Pass, Ogre::ColourValue>(
				talkColourPass, Ogre::ColourValue::ZERO, time, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
			talkNode->setScale( talkScale,talkScale,talkScale );
			registerTween(new TCos<Ogre::SceneNode, Ogre::Vector3>(
				talkNode, Ogre::Vector3::ZERO, time, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		}
	}

	void Animal::chooseBasic() {

		if (idle) {
			currAnimation = &anims[basic];
			return;
		}

		while (true) { /// repeat until an available animation is found

			double random = (double)rand() / RAND_MAX;

			for (std::map<AnimalAnim*, double>::iterator it = basicLoops.begin(); it != basicLoops.end(); ++it) {
				if (random <= it->second) {
					/// random value is within these iterator's "limits"
					if (currAnimation == it->first && !currAnimation->repeatable) break; /// retry!
					currAnimation = it->first;
					return;
				}
			}
		}
	}

	void Animal::run(double timeToRun) {

		/// run <skeletal>
		double time = timeToRun;
		do {
			time = currAnimation->run(time);
			if (time > 0) {
				/// current animation finished...
				currAnimation->reset();

				/// and there's time left to run the next one
				if (nextAnimation == Animal::_none) {
					chooseBasic(); /// set 'currAnimation'
				} else {
					currAnimation = &anims[nextAnimation];
					if (!currAnimation->_volatile) { /// must take facials away! (facials won't return later cause I'M LAZY!!!!)
						isSmiling = false;
						isAnxious = false;
						fadeoutFacials(0); /// fade all
						/// <NOTE> : I could make them <return> by having a 'nextFacial' property, which will start if previous anim wasn't volatile but current IS!
						/// was: anims[rest].reset(); /// take out the "hack"
					}
					nextAnimation = _none;
				}

				currAnimation->play(animalNode->_getDerivedPosition());
			}
		} while (time > 0);

		/// run <facial>
		for (int i = facialPlaying.size() - 1; i >= 0; --i) {
			if (facialPlaying[i]->run(timeToRun)) {
				facialPlaying[i]->reset();
				facialPlaying.erase(facialPlaying.begin() + i);
			}
		}
	}

	double Animal::fadeoutAnim(AnimalAnim* anim) {
		if (!anim->_volatile) return 0;

		/// check if animation will finish prior to standard fade-out time
		double fadeTime;
		if (!anim->pauseAtEnd && Animal::fadeTime > anim->remainingRuntime()) {
			fadeTime = (1 - anim->animStates[0]->getWeight()) / anim->remainingRuntime();
		} else {
			fadeTime = Animal::fadeSpeed;
		}
		anim->weightSpeed = -fadeTime;
		return fadeTime;
	}

	void Animal::fadeinRest(double fadeSpeed) {
		AnimalAnim* animRest = &anims[rest];
		for (unsigned int i = 0; i < animRest->animStates.size(); ++i) {
			Ogre::AnimationState* animState = animRest->animStates[i];
			animState->setWeight(0);
			animState->setEnabled(true);
		}
		animRest->weightSpeed = fadeSpeed;
		addFacial(animRest);
	}

	void Animal::event(P3D::Animal::Animation animID) {

		AnimalAnim* anim;

		switch (animID) {

			case happy1:
				if (isSmiling || isAnxious) return;
				addFacial(&anims[happy1]);
				return;

			case happy2:
			case happy3:
			case sad:
			case win:
			case lose:
				if (nextAnimation < animID) {
					/// if current animation is "volatile" fade-it-out and fade-in the "rest" pose (if not already fading-in)
					if (currAnimation->_volatile && nextAnimation == _none) {
						fadeinRest( fadeoutAnim(currAnimation) - 0.0001); /// <HACK>? : 'fadeinRest' needs to be 1 frame slower than 'fadeoutAnim' <!!!>
					}
					nextAnimation = animID;
				}
				return;

			case even:
				isSmiling = false;
				isAnxious = false;
				fadeoutFacials(&anims[happy1]); /// fade all except 'happy1' (i.e smiling & anxious) <!>
				return;

			case smiling:
				if (isSmiling) return;
				isSmiling = true;
				isAnxious = false;
				anim = &anims[smiling];
				fadeoutFacials(anim);
				addFacial(anim);
				return;

			case anxious:
				if (isAnxious) return;
				isSmiling = false;
				isAnxious = true;
				anim = &anims[anxious];
				fadeoutFacials(anim);
				addFacial(anim);
				return;

			case _none: /// debug!!!
				start();
				return;

			default:
				nextAnimation = animID;
				return;
		}
	}

	void Animal::addFacial(AnimalAnim* anim) {
		/// don't add a facial if it's already playing (even if fading-out!! CAUSE I'M LAZY!!!)
		for (int i = facialPlaying.size() - 1; i >= 0; --i) {
			if (facialPlaying[i] == anim) return;
		}
		anim->play(animalNode->_getDerivedPosition());
		facialPlaying.push_back(anim);
	}

	void Animal::fadeoutFacials(AnimalAnim* anim) {
		for (int i = facialPlaying.size() - 1; i >= 0; --i) {
			if (facialPlaying[i] != anim) fadeoutAnim(facialPlaying[i]);
		}
	}
}