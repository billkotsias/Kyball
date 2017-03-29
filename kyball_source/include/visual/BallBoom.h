////////////
// BallBoom - Billboard visual element
////////////

#pragma once
#ifndef BallBoom_H
#define BallBoom_H

#include "visual\Billboard.h"
#include "tween\TPlayed.h"

#include "game\form\BallState.h"
#include <OgreColourValue.h>
#include <vector>

namespace P3D {

	class GameplayBase;
	class TPlaylist;

	class BallBoom : public Billboard, public TPlayed {

	protected:

		/// <BallState::BallType> to <Ogre::ColourValue> conversion table
		static const std::vector<Ogre::ColourValue> ballColours;

	public:

		// constructor
		// => boss
		//	  start frame
		//	  end frame
		//	  current frame
		//	  frame step
		//	  time to live
		BallBoom(GameplayBase* base, BallState::BallType ballType, Ogre::Vector3 pos, double _ttl = Infinite);
		virtual ~BallBoom();

	};
}

#endif