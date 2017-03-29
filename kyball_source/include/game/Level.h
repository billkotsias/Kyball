// ----------------------------------------
// Level - a layer above GameplayCollection
// ----------------------------------------

#pragma once
#ifndef Level_H
#define Level_H

#include "game\GameplayCollection.h"
#include <vector>

namespace P3D {

	class Particles;

	class Level : public GameplayCollection {

	public:

		Level() {}
		virtual ~Level();

		void addParticles(Particles* _particles);

	private:

		std::vector<Particles*> particles;
	};

}

#endif