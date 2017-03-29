#include <StdAfx.h>

#include "Particles.h"
#include "OgreBuilder.h"
#include "game\Level.h"

namespace P3D {

	Level::~Level()
	{
		for (int i = particles.size() - 1; i >= 0; --i) {
			delete particles.at(i);
		}
		OgreBuilder::unsetFog(); /// simply unset the fog 'LevelBuilder' set earlier!
	}

	void Level::addParticles(Particles* _particles) {
		particles.push_back(_particles);
	}
}