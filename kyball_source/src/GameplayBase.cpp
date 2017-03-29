#include "StdAfx.h"
#include "game\GameplayBase.h"

#include "SceneBaseCreator.h"
#include "game\rule\RuleManager.h"
#include "game\cannon\Cannon.h"
#include "game\Camera.h"
#include "game\net\Net.h"
#include "game\animal\Animal.h"
#include "game\event\EventManager.h"
#include "game\GameplayCollection.h"
#include "game\GameplayObject.h"
#include "game\form\Form.h"
#include "game\ai\AI.h"
#include "game\ai\AIPVAnimator.h"
#include "ShadowManager.h"
#include "BurstBoss.h"
#include "visual\VisualBoss.h"
#include "gui\SubScore.h"

#include "core\Utils.h"
#include <OgreSceneNode.h>
#include <OgreBillboardSet.h>

namespace P3D {

	//
	// GameplayBase
	//

	const std::string GameplayBase::BBSET_NAME = "BBSet";

	GameplayBase::GameplayBase(unsigned int id, Ogre::SceneNode* node, MapOptions::CosmosID cosm, Cannon* can, Camera* cam, Net* inNet, Animal* anim,
		const Ogre::Vector3& start, const Ogre::Vector3& end, GameplayCollection* coll, ShadowManager* shad, AIPVAnimator* inAipva, BurstBoss* bur,
		const Ogre::Vector3& cube) :

		gameplayRootNode(node), cosmosID(cosm), cannon(can), camera(cam), ballStartPos(start), ballEndPos(end), net(inNet), animal(anim), collection(coll),
		shadowManager(shad), aipva(inAipva), burst(bur), cubeSize(cube) {

			ID = FANLib::Utils::toString(id);
			cosmos = MapOptions::getCosmosStrFromID(cosmosID);

			eventManager = new EventManager();
			ruleManager = new RuleManager();
			subScore = new SubScore(this, ID);

			insertGameplayObjects(&collection->gameplayObjects); /// insert my <own> GameplayObjects
	}

	GameplayBase::~GameplayBase() {

		// UNFORTUNATELY THE ORDER HERE IS VERY IMPORTANT BECAUSE OF GREAT INTERDEPENDENCE. THAT'S SOMETHING I MUST TAKE CARE OF IN THE FUTURE !!!

		/// delete 'cannon' and 'camera' <1st> because they use nodes found in 'collection'
		delete cannon;
		cannon = 0;

		delete camera;
		camera = 0;

		delete animal;
		animal = 0;

		/// delete <own> collection <2nd>
		eraseGameplayObjects(&collection->gameplayObjects);
		delete collection;
		collection = 0;

		/// misc
		delete net;
		net = 0;

		delete shadowManager;
		shadowManager = 0;

		delete burst;
		burst = 0;

		delete aipva;
		aipva = 0;

		delete subScore;
		subScore = 0;

		/// managers
		delete eventManager;
		eventManager = 0;

		delete ruleManager;
		ruleManager = 0;

		/// BBSets must be deleted <AFTER> all collections (why?) !
		for (std::map<BBSet, Ogre::BillboardSet*>::iterator it = BBSets.begin(); it != BBSets.end(); ++it) {
			destroyBBSet(it->second);
		}
		BBSets.clear();

		/// debug : check if there are any left-overs in gameplayObjects and produce an error!
#ifndef	_DEPLOY
		if (!gameplayObjects.empty()) throw "Base not empty!!! Who forgot to clean-up his GameplayObjects ???";
#endif

		// was : gameplayRootNode->removeAndDestroyAllChildren();
		//gameplayRootNode->getParentSceneNode()->removeAndDestroyChild(gameplayRootNode->getName());
	}

	void GameplayBase::insertGameplayObjects(std::deque<GameplayObject*>* dequePtr) {
		gameplayObjects[dequePtr] = true;
	}

	void GameplayBase::eraseGameplayObjects(std::deque<GameplayObject*>* dequePtr) {
		gameplayObjects.erase(dequePtr);
	}

	Ogre::BillboardSet* GameplayBase::newBBSet(std::string name, double sizeX, double sizeY, int xDiv, int yDiv, int poolSize, bool accu) {

		/// - ensure unique name for every set created
		std::string setName = BBSET_NAME + "_" + name + ID;

		/// - create 'BillboardSet'
		Ogre::BillboardSet* bbset;
		bbset = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createBillboardSet(setName, poolSize);

		bbset->setMaterialName(name);
		if (bbset->getMaterial()->isTransparent()) bbset->setSortingEnabled(true); /// 'cause Ogre plays dumb
		bbset->setDefaultDimensions(sizeX, sizeY);
		bbset->setBillboardRotationType(Ogre::BBR_VERTEX);	/// vertex rotation
		bbset->setTextureStacksAndSlices(yDiv, xDiv);		/// graphics slices
		bbset->setUseAccurateFacing(accu);					/// essential when <close> to billboards!

		return bbset;
	}

	void GameplayBase::destroyBBSet(Ogre::BillboardSet* bbset) {
		bbset->clear();				/// that's the must useful function here (if at all!)
		bbset->detatchFromParent();	/// this is probably automatically done <!>
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->destroyBillboardSet(bbset);
	}

	Ogre::BillboardSet* GameplayBase::setBBSet(GameplayBase::BBSet id, std::string name, double sizeX, double sizeY, int xDiv, int yDiv, int poolSize, bool accu) {
		Ogre::BillboardSet* bbset;

		/// remove previous
		bbset = getBBSet(id);
		if (bbset) destroyBBSet(bbset);

		/// create new
		bbset = newBBSet(name, sizeX, sizeY, xDiv, yDiv, poolSize, accu);
		BBSets[id] = bbset;

		return bbset;
	}

	Ogre::BillboardSet* GameplayBase::getBBSet(P3D::GameplayBase::BBSet id) {
		std::map<GameplayBase::BBSet, Ogre::BillboardSet*>::iterator it = BBSets.find(id);
		if (it != BBSets.end()) {
			return it->second;
		}
		return 0; /// not found
	}

	void GameplayBase::groupCollisionObjects() {

		GameplayObjectsIterator gameIterator;
		GameplayObject* gameplayObject;

		causeCollision.clear();
		acceptCollision.clear();
		hybridCollision.clear();

		if (gameIterator.begin(this, gameplayObject)) {
			while (gameIterator.next()) {
				switch (gameplayObject->ai->checkForCollision()) {
					case AI::CAUSE:
						causeCollision.push_back(gameplayObject);
						break;
					case AI::ACCEPT:
						acceptCollision.push_back(gameplayObject);
						break;
					case AI::HYBRID:
						hybridCollision.push_back(gameplayObject);
						break;
				}
			}
		}
	}

	void GameplayBase::prepareCollisionObjects() {
		GameplayObjectsIterator gameIterator;
		GameplayObject* gameplayObject;

		gameIterator.begin(this, gameplayObject); /// if there are colliding objects, there ARE objects to start with
		while (gameIterator.next()) {
			gameplayObject->prepareCollisionObject();
		}
	}


	//
	// GameplayObjectsIterator
	//

	bool GameplayObjectsIterator::begin(GameplayBase* base, GameplayObject*& objPtr) {
		map = &(base->gameplayObjects);
		iterator = map->begin();
		if (iterator == map->end()) return false;
		currentDequeIndex = -1;
		currentGameplayObject = &objPtr;

		return true;
	}

	bool GameplayObjectsIterator::next() {

		++currentDequeIndex;
		while ((unsigned)currentDequeIndex >= iterator->first->size()) { /// is current deque exhausted?
			++iterator;

			if (iterator == map->end()) {
				/// the whole map is exhausted
				iterator = map->begin(); /// reset
				currentDequeIndex = -1;
				return false;
			}

			currentDequeIndex = 0;
		}

		(*currentGameplayObject) = iterator->first->at(currentDequeIndex);
		return true;
	}

	void GameplayObjectsIterator::removeGameplayObject() {
		if (currentDequeIndex < 0) return; /// can't be tricked!
		iterator->first->erase(iterator->first->begin() + currentDequeIndex--); /// <NOTE> : currentDequeIndex--
	}


}