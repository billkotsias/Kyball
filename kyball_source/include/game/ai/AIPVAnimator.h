// ------------.................................
// AIPVAnimator - Animator of 'AIPosVel' objects
// ------------`````````````````````````````````

#pragma once
#ifndef AIPVAnimator_H
#define AIPVAnimator_H

#include <map>

namespace Ogre {
}

namespace P3D {

	class AIPVAnime;
	class AIPosVel;
	class AIPVListener;

	/* AIPVAnime parameters */;
	class AIPVAParams {

	public:

		AIPVListener* listener;
		double delay; /// delay till start of animation (in secs)

		AIPVAParams(double d = 0, AIPVListener* l = 0) : delay(d), listener(l) {
		};

	};

	/* Animator */;
	class AIPVAnimator {

	private:

		std::map<AIPosVel*, std::map<AIPVAnime*, AIPVAParams> > animes;

	public:

		~AIPVAnimator();

		/// add new animation
		/// =>	animation
		///		delay
		///		listener
		void newAnime(AIPVAnime*, double = 0, AIPVListener* = 0);
		void deleteAnime(AIPVAnime*);
		void deleteAllAnimes(AIPosVel*); /// delete this object's all Animes
		
		void run();
	};

}

#endif