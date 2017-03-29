#include "StdAfx.h"
#include "game\GameplayCollection.h"

// although seemingly lightweight, this class is in fact "heavy" in order to do full self-cleaning
#include "game\GameplayObject.h"
#include "visual\Visual.h"
#include "visual\VisualBoss.h"
#include "SceneBaseCreator.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace P3D {

	GameplayCollection::~GameplayCollection() {

		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();

		/// delete all stored items; the order is <important> !!!

		/// delete <visuals> 1st!
		for (std::list<Visual*>::reverse_iterator rit = visuals.rbegin(); rit != visuals.rend(); ++rit) {
			delete (*rit);
		}
		visuals.clear();

		/// delete <animations> (corresponding animation states - if any left - are automagically destroyed)
		for (int i = ogreAnimations.size() - 1; i >= 0 ; --i) {
			sceneMan->destroyAnimation( ogreAnimations.at(i)->getName() );
		}
		ogreAnimations.clear();

		/// delete <gameplay objects> (reverse order)
		for (int i = gameplayObjects.size() - 1; i >= 0 ; --i) {
			delete gameplayObjects.at(i);
		}
		gameplayObjects.clear();

		/// delete <decor> (reverse order)
		for (int i = movables.size() - 1; i >= 0 ; --i) {
			sceneMan->destroyMovableObject(movables.at(i));
		}
		movables.clear();

		/// delete <nodes> last ! Must also be in reverse order!!!
		for (int i = nodes.size() - 1; i >= 0 ; --i) {
			Ogre::SceneNode* node = nodes[i];
			node->detachAllObjects();			/// just in case
			sceneMan->destroySceneNode(node);
		}
		nodes.clear();

	}

	Ogre::SceneNode* GameplayCollection::newSceneNode(Ogre::SceneNode *parent, const Ogre::Vector3 *pos) {
		Ogre::SceneNode* node = parent->createChildSceneNode(*pos);
		nodes.push_back(node); /// <store> it
		return node;
	}

	void GameplayCollection::pauseVisuals() {
		VisualBoss* const boss = VisualBoss::getSingletonPtr();
		for (std::list<Visual*>::iterator it = visuals.begin(); it != visuals.end(); ++it) boss->pause(*it);
	}

	void GameplayCollection::unpauseVisuals() {
		VisualBoss* const boss = VisualBoss::getSingletonPtr();
		for (std::list<Visual*>::iterator it = visuals.begin(); it != visuals.end(); ++it) boss->unpause(*it);
	}

}