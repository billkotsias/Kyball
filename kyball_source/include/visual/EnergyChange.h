///////////////
// EnergyChange - visual element
///////////////

#pragma once
#ifndef EnergyChange_H
#define EnergyChange_H

#include "visual\Visual.h"
#include "tween\TPlayed.h"

namespace P3D {

	class EnergyChange : public Visual, public TPlayed {

	protected:

		Ogre::SceneNode* node;
		Ogre::MaterialPtr myMat;
		bool timeToDie;
		double timeToLive;
		double totalTime;

		//void playShit();
		Ogre::Vector3 pos;

	public:

		// constructor
		// =>	a SceneNode to attach myself to; just about anything will suffice
		//		world position
		//		scale
		//		time-to-live (ttl)
		//		ttl randomness (per cent)
		//		local orientation
		//		play sound?
		//		delay (mainly for trailer purposes)
		EnergyChange(Ogre::SceneNode* parent, const Ogre::Vector3& pos, const Ogre::Vector3& scale, double ttl, double rnd,
			const Ogre::Quaternion& rot, bool playSound, double delay = 0);
		virtual ~EnergyChange();

		// overrides
		virtual bool run(double);
		void playlistFinished(TPlaylist* list);

	};
}

#endif