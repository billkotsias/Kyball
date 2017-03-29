#include "StdAfx.h"
#include "visual\Ambience.h"

#include "visual\VisualBoss.h"
#include "OgreBuilder.h"
#include "sound\Sound.h"

namespace P3D {

	Ambience::Ambience(std::vector<std::string> _names, Ogre::SceneNode* parent, Ogre::Vector3 _center, double inner, double outer, double _height, double _interval, double _random, bool _pause) : playingSoundHandle(0), center(_center), innerRadius(inner), outerRadius(outer), height(_height), interval(_interval), random(_random), pause(_pause)
	{
		for (unsigned int i = 0; i < _names.size(); ++i) {
			const std::string& soundName = _names.at(i);
			Sound* sound = Sound::getScripted( soundName.c_str(), "<ambience>" );
			sound->priority = false; /// all ambient sounds have low priority by default <!>
			sounds.push_back( sound );
		}

		origin = parent->createChildSceneNode();
		timeToNextSound = calcNewTime(0.5, 0.5);
		shuffle();

		VisualBoss::getSingletonPtr()->insert(this);
	}

	Ambience::~Ambience() {
		if (playingSoundHandle) {
			//std::cout << "Ambience dying\n";
			Sound::stop(playingSoundHandle);
		}
		OgreBuilder::destroySceneNode(origin);
	}

	void Ambience::shuffle() {

		static const unsigned int TIMES_TO_SHUFFLE = 2;

		/// prepare array
		unsigned int count = sounds.size();
		std::vector<unsigned int> nums;
		for (unsigned int i = 0; i < count; ++i) {
			nums.push_back(i);
		}
		/// shuffle array
		for (unsigned int i = 0; i < count * TIMES_TO_SHUFFLE; ++i) {
			unsigned int i1 = count * rand() / (RAND_MAX + 1);
			unsigned int i2 = count * rand() / (RAND_MAX + 1);
			unsigned int temp = nums.at(i1);
			nums.at(i1) = nums.at(i2);
			nums.at(i2) = temp;
		}
		/// get shuffled sounds
		for (unsigned int i = 0; i < count; ++i) {
			shuffled.push_back( sounds.at(nums.at(i)) );
		}
	}

	double Ambience::calcNewTime(double weight, double randomness) {
		static const double randMax = RAND_MAX / 2.;
		return interval * (weight + randomness * (randMax - rand()) / randMax);
		//return interval * (1 + random * (randMax - rand()) / randMax);
	}

	bool Ambience::run(double time)
	{
		if (playingSoundHandle) {
			if (Sound::isPlaying(playingSoundHandle)) return false;
			playingSoundHandle = 0;
		}

		timeToNextSound -= time;
		if (timeToNextSound <= 0)
		{
			timeToNextSound = calcNewTime(1., random);
			if (shuffled.empty()) {
				if (pause) return false; /// stop producing sounds!
				shuffle(); /// refill
			}
			Sound* sound = shuffled.back();
			shuffled.pop_back();
			playingSoundHandle = sound->play( getRandomPosition() );
		}

		return false;
	}

	Ogre::Vector3 Ambience::getRandomPosition()
	{
		double distance = innerRadius + (outerRadius - innerRadius) * rand() / RAND_MAX;	/// x/y-distance from center
		double angle = Ogre::Math::TWO_PI * rand() / (RAND_MAX + 1);						/// direction from center
		double y = height * rand() / RAND_MAX;												/// z-distance from center
		origin->setPosition( center.x + distance * cos(angle), center.y + y, center.z + distance * sin(angle));
		return origin->_getDerivedPosition();
	}
}