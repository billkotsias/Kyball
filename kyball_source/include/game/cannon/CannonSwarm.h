// -----------
// CannonSwarm - a special visual element for 'Cannon'
// -----------

#pragma once
#ifndef CannonSwarm_H
#define CannonSwarm_H

#include <OgreMaterial.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <map>
#include "tween\TPlayed.h"

namespace Ogre {
	class SceneNode;
	class Entity;
	class Pass;
}

namespace P3D {

	class TPlaylist;

	class CannonSwarm : public TPlayed {

	private:

		Ogre::Entity* entity;
		Ogre::SceneNode* node;
		Ogre::MaterialPtr material;
		Ogre::Pass* pass; /// material pass 0

#ifdef _DEBUG
		TPlaylist* fin;
		TPlaylist* got;		
		virtual void playlistFinished(TPlaylist*);
#endif

	public:

		// => unique name
		//	  parent SceneNode
		CannonSwarm(const std::string&, Ogre::SceneNode*);
		~CannonSwarm();

		// => world position
		//	  fade to this colour
		//	  fade from this scale...
		//	  ... to this scale
		//	  time to get there
		void fadeIn(const Ogre::Vector3&, const Ogre::ColourValue&, double, double, double);

		// => world position
		//	  time to get there
		//	  time to scale down
		void gotoLaunch(const Ogre::Vector3&, double, double);
	};

}

#endif