///////////
// AIPosVel - AIPos with "velocity"
///////////

#pragma once
#ifndef AIPosVel_H
#define AIPosVel_H

#include "AIPos.h"

namespace P3D {

	class AIPVAnime;

	class AIPosVel : public AIPos {

	protected:

		Ogre::SceneNode* vel;	/// velocity of AI in world

	public:

		// constructor
		// => ai "owner"
		//	  parent = parent node that holds my position node
		//	  position = position of AI relative to parent
		//	  is this object moving towards cannon?
		AIPosVel(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&, bool mover);
		virtual ~AIPosVel();

		virtual void setVelocity(const Ogre::Vector3&);	/// set AI's <local> velocity
		const Ogre::Vector3& getVelocity();				/// get AI's <local> velocity

		// "link"/"unlink" requests to supporting rules (like 'MatchThree'); only available from 'AIPosVel' and up!
		void link();
		void unlink();

		// overrides
		virtual void run(double);
		virtual bool attachForm(Form*); /// attach 'Form' to my "position"
	};
}

#endif