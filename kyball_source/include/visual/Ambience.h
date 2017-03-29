////////////////////////////
// Ambience - ambient sounds
////////////////////////////

#pragma once
#ifndef Ambience_H
#define Ambience_H

#include "visual\Visual.h"
#include <vector>

namespace P3D {

	class Sound;

	// Full Ambience management for a game level
	class Ambience : public Visual {

	protected:

		std::vector<Sound*> sounds; /// all sounds to play

		Ogre::SceneNode* origin;/// helper node for sound-positioning
		Ogre::Vector3 center;	/// center of pipe's base
		double innerRadius;		/// sound can't be inside this circle
		double outerRadius;		/// sound can't be outside this circle
		double height;			/// may be negative

		double interval;			/// time among each sound played
		double random;				/// interval randomization : [0=none...1=max]
		bool pause;

		std::vector<Sound*> shuffled;
		double timeToNextSound;
		unsigned int playingSoundHandle;

		void shuffle();
		double calcNewTime(double weight, double randomness);

		Ogre::Vector3 getRandomPosition();

	public:

		Ambience(std::vector<std::string> _names, Ogre::SceneNode* parent, Ogre::Vector3 _center, double inner, double outer, double _height, double _interval, double _random, bool _pause);
		~Ambience();

		virtual bool run(double);
	};
}

#endif