// ----------------------------------------------------
// LevelBuilder - build custom level for current cosmos
// ----------------------------------------------------

#pragma once
#ifndef LevelBuilder_H
#define LevelBuilder_H

#include "map\MapOptions.h"
#include <deque>

namespace Ogre {
	class SceneManager;
	class SceneNode;
}

namespace FANLib {
	class FSLClass;
}

namespace P3D {

	class Queue;
	class HintBox;
	class Level;
	class Player;
	class GameplayBase;

	class LevelBuilder {

	public:
		LevelBuilder();	/// constructor/destructor
		~LevelBuilder();

		// build a level for 'Gameplay' from script; update 'GameplayBase' with Level's scripted "rules"
		/// Rules include :	- what kind of <BallCreator> the <Cannon> has
		///					- what's the <level-end> condition
		///					- what's the <ball-stuck> consequence (e.g when do stuck balls explode)
		Level* build(unsigned levelNum, GameplayBase*, Queue*, HintBox*);

	};
}

#endif