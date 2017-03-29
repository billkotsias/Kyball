////////
// AIPos - AI with a "position" in world
////////

#pragma once
#ifndef AIPos_H
#define AIPos_H

#include "AI.h"
#include <OgreVector3.h>

namespace P3D {

	class AIPos : public AI {

	protected:

		Ogre::SceneNode* pos; /// position of AI in world

	public:

		// constructor
		// => ai "owner"
		//	  parent = parent node that holds my position node
		//	  position = position of AI relative to parent
		AIPos(GameplayObject*, Ogre::SceneNode*, const Ogre::Vector3&);
		virtual ~AIPos();

		virtual void setPosition(const Ogre::Vector3&);		/// set AI's <local> position
		virtual void setOrientation(const Ogre::Quaternion&);	/// set AI's <local> orientation

		virtual void setWorldPosition(const Ogre::Vector3&); /// set AI's <world> position

		virtual const Ogre::Vector3& getPosition();			/// get AI's <local> position
		virtual const Ogre::Quaternion& getOrientation();	/// get AI's <local> orientation

		// overrides
		virtual Ogre::Vector3 getWorldPosition();
		virtual bool attachForm(Form*); /// attach 'Form' to my "position"
	};
}

#endif