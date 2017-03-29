// Simple Particles management

#pragma once
#ifndef Particles_H
#define Particles_H

#include <string>
#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
	class ParticleSystem;
}

namespace P3D {

	class Particles {

	private:

		Ogre::ParticleSystem* system;

	public:

		static const std::string LINEAR_FORCE;
		static void ScaleSystem(Ogre::ParticleSystem* system, double scale);

		Particles(const std::string& systemName, Ogre::SceneNode* parent, Ogre::Vector3& position, double scale, double fastForward, bool shadows);
		~Particles();
	};
}

#endif