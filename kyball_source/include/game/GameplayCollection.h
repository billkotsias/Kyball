// ----------------------------------------------------------------
// GameplayCollection - one of several groups of 'Gameplay' objects
// ----------------------------------------------------------------
/// contains :
/// - SceneNodes		}
/// - MovableObjects	} to be destroyed altogether (if any left)
/// - GameplayObjects	}

#pragma once
#ifndef GameplayCollection_H
#define GameplayCollection_H

#include <deque>
#include <list>

namespace Ogre {
	class SceneNode;
	class MovableObject;
	class Vector3;
	class Animation;
}

namespace P3D {

	class GameplayObject;
	class Visual;

	class GameplayCollection {

	public:

		std::deque<Ogre::SceneNode*>		nodes;
		std::deque<Ogre::MovableObject*>	movables;
		std::deque<GameplayObject*>			gameplayObjects;

		/// the objects stored here should <NEVER> ask to be deleted by the <VisualBoss> (by returning 'true' from 'run()') <!>
		/// ... or they should remove THEMSELVES from the list before asking VisualBoss to kill'em up!
		std::list<Visual*>					visuals;

		std::deque<Ogre::Animation*>		ogreAnimations;

		// constructor
		GameplayCollection() {};
		virtual ~GameplayCollection();

		/// create and store a node in the collection in 1 move
		Ogre::SceneNode* newSceneNode(Ogre::SceneNode*, const Ogre::Vector3*);

		/// store an item (and return it for convenience)
		inline Ogre::MovableObject* storeMovable(Ogre::MovableObject* obj) {
			movables.push_back(obj);
			return obj;
		}

		inline GameplayObject* storeGameplayObject(GameplayObject* obj) {
			gameplayObjects.push_back(obj);
			return obj;
		}

		inline Visual* storeVisual(Visual* vis) {
			visuals.push_back(vis);
			return vis;
		}

		inline Ogre::Animation* storeAnimation(Ogre::Animation* vis) { ogreAnimations.push_back(vis); return vis; }

		// some helping hands; please leave here
		void pauseVisuals();
		void unpauseVisuals();
	};

}

#endif