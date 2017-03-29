/////////
// Animal - cheerleader
/////////

#pragma once
#ifndef Animal_H
#define Animal_H

//#include "AnimStruct.h"

#include <string>
#include <vector>
#include <map>
#include <tween\TPlayed.h>

namespace Ogre {
	class Entity;
	class AnimationState;
}

namespace P3D {

	class Sound;
	class BurstBoss;

	class AnimalAnim {
	public:

		std::vector<Ogre::AnimationState*> animStates; /// support for >1 animations (of same length!) in case one is consisted of more (e.g skeletal + morph)
		double speed;		/// default = 1.0
		double weightSpeed;	/// default = 0
		bool _volatile;		/// can this anim be "faded-out"?
		bool pauseAtEnd;	/// if true, anim doesn't "end", instead it stays paused when it finishes
		bool repeatable;	/// can this animation be repeated right away after it's finished? (only used in <chooseBasic()>)

		// particles!
		unsigned particleSystem;
		std::string particleBone;
		double particleDelay;
		double currentDelay;
		Ogre::Entity* entity; /// required to attach "bursts" to its bones
		BurstBoss* burst;

		// constructor
		AnimalAnim();

		// start playing
		// => world position of animal
		void play(const Ogre::Vector3&);

		// play animation
		// <= non-zero = time remaining after animation has finished
		double run(double);

		// set accompanying sound
		Sound* sound;
		unsigned int soundHandle;
		void setSound(const std::string&);

		// reset animations
		void reset();

		/// getters
		double remainingRuntime();
	};

	class Animal : public TPlayed
	{

	public:

		/// animation id/event combo
		enum Animation {

			_none = -1, /// indicates "no animation"

			/// skeletal "rest" : this is a one-frame animation to which volatile animations are blended, because it's <different> to "no skeletal effect"! Ask Alexis!!!
			rest,

			/// skeleton anims : mutually exclusive
			basic,	/// }
			tick1,	/// }
			tick2,	/// } auto-played; volatile!
			tick3,	/// }
			look,	/// }

			/// the <order> is important : greater wins the "nextAnimation" place
			happy1,	/// }
			happy2,	/// }
			happy3,	/// } event-driven
			sad,	/// }
			win,	/// }
			lose,	/// }

			/// facial anims : added to skeleton anims
			even,		/// } (= no facial)	
			smiling,	/// } event-driven
			anxious,	/// }

			_last,		/// enumerates number of events
		};

	protected:

		static void buildOnce();
		static std::vector<Animal::Animation> enumToAnim;
		static std::map<Animal::Animation, std::string> animToString;
		static const char* const ANIM_MORPH_POSTFIX;

		std::vector<AnimalAnim> anims;				/// <anims> iterator is <Animal::Animation>-compatible
		std::map<AnimalAnim*, double> basicLoops;	/// basic loop animations
		//std::vector<AnimalAnim*> facial;			/// facial (additive) animations : NO NEED TO GROUP'EM UP

		/// animations being played
		AnimalAnim* currAnimation;			/// skeletal animation being played
		Animal::Animation nextAnimation;	/// skeletal animation (id) to be played right after current one
		std::vector<AnimalAnim*> facialPlaying; /// facial animations being played
		bool isSmiling;
		bool isAnxious;

		Ogre::SceneNode* animalNode;
		double talkScale;
		Ogre::ColourValue talkColour;
		Ogre::Pass* talkColourPass;

	public:

		static Animal::Animation FromFSLEnum(int i) {
			return enumToAnim[i];
		};

		static double fadeSpeed;
		static double fadeTime;

		// constructor
		// => according Entity
		//	  cosmos ID string
		Animal(Ogre::Entity*, const std::string&, BurstBoss* burst);
		~Animal();

		// when there are no external events, does animal make any other animations besides 'idle'?
		bool idle;

		// start animating/reset
		void start();

		// choose a random basic loop
		void chooseBasic();

		// time to run
		void run(double);

		// alter behaviour or cause an out-of-loop animation
		void event(Animal::Animation);

		// set fadeout parameters to a non-volatile 'AnimalAnim'
		// <= actual time till full fade-out (calculated!)
		double fadeoutAnim(AnimalAnim*);

		// fade-in "rest" skeletal pose
		// => time to fade-in
		void fadeinRest(double);

		// start a new facial
		void addFacial(AnimalAnim*);

		// fadeout all but one facial
		void fadeoutFacials(AnimalAnim*);

		// also sets idle = true
		void talk(bool enable);

		Ogre::SceneNode* getAnimalNode() { return animalNode; };
	};
}

#endif