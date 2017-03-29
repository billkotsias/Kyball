// ------------
// GameplayBase - a structure holding all Gameplay-required info
// ------------

#pragma once
#ifndef GameplayBase_H
#define GameplayBase_H

#include "map\MapOptions.h"

#include <map>
#include <deque>
#include <string>
#include <OgreVector3.h>

namespace Ogre {
	class BillboardSet;
	class SceneNode;
}

namespace P3D {

	class Cannon;
	class Camera;
	class GameplayObject;
	class Net;
	class EventManager;
	class RuleManager;
	class GameplayCollection;
	class ShadowManager;
	class AIPVAnimator;
	class Animal;
	class BurstBoss;
	class BallAI;
	class SubScore;
	class Player;

	class GameplayBase {

	public:

		/// Pre-defined BillboardSets
		enum BBSet {
			BALLS,
			BALLS_ALPHA,
			AIM,
			AIM_HELP,
			BALL_BOOM,
			BOBS,
		};

	private:

		std::map<std::deque<GameplayObject*>*, bool> gameplayObjects; /* holds 'GameplayObject's from various classes (mine too!!!) */;
		/// above objects are separated according to their 'checkCollision' status :
		std::vector<GameplayObject*> causeCollision;	/// objects causing collision
		std::vector<GameplayObject*> acceptCollision;	/// objects accepting collision
		std::vector<GameplayObject*> hybridCollision;	/// objects causing collision but not accepting from other hybrid

		std::map<GameplayObject*, bool> balls;			/// keep all created balls (gameplay objects with BallAI; for <BallCreator> reference)

		std::map<GameplayBase::BBSet, Ogre::BillboardSet*> BBSets;

		/// per-base properties
		std::string			ID;				/// unique base id
		Ogre::SceneNode*	gameplayRootNode;
		std::string			cosmos;
		Cannon*				cannon;
		Camera*				camera;
		Ogre::Vector3		ballStartPos;	/// position of 'Ball' when created
		Ogre::Vector3		ballEndPos;		/// position of 'Ball' in 'Cannon'
		Net*				net;			/// net of nodes defining positions of "stuck" balls
		Animal*				animal;			/// animal-avatar
		GameplayCollection*	collection;		/// has-a collection (for cleaning-up)
		ShadowManager*		shadowManager;
		AIPVAnimator*		aipva;
		BurstBoss*			burst;
		Ogre::Vector3		cubeSize;
		MapOptions::CosmosID cosmosID;

		/// non-inherited classes, thus constructor-created...
		EventManager*		eventManager;	/// transmits gameplay events to all interested
		RuleManager*		ruleManager;
		SubScore*			subScore;

	public:

		/* constructor */;
		GameplayBase(
			unsigned int, Ogre::SceneNode*, MapOptions::CosmosID, Cannon*, Camera*, Net*, Animal*, const Ogre::Vector3&, const Ogre::Vector3&, GameplayCollection*,
			ShadowManager*, AIPVAnimator*, BurstBoss*, const Ogre::Vector3&);

		~GameplayBase();

		unsigned int score;	/// <!!!> current level's score! Accessible by everyone!
		Player* player;		/// the player, too
		int levelZ;			/// the level's depth, too

		void insertGameplayObjects(std::deque<GameplayObject*>*);
		void eraseGameplayObjects(std::deque<GameplayObject*>*);

		inline std::string			getID()				{ return ID; };
		inline Ogre::SceneNode*		getRootNode()		{ return gameplayRootNode; };
		inline MapOptions::CosmosID	getCosmosID()		{ return cosmosID; };
		inline std::string			getCosmos()			{ return cosmos; };
		inline Cannon*				getCannon()			{ return cannon; };
		inline Camera*				getCamera()			{ return camera; };
		inline Net*					getNet()			{ return net; };
		inline Animal*				getAnimal()			{ return animal; };
		inline Ogre::Vector3		getBallStartPos()	{ return ballStartPos; };
		inline Ogre::Vector3		getBallEndPos()		{ return ballEndPos; };
		inline EventManager*		getEventManager()	{ return eventManager; };
		inline GameplayCollection*	getCollection()		{ return collection; };
		inline ShadowManager*		getShadowManager()	{ return shadowManager; };
		inline AIPVAnimator*		getAIPVAnimator()	{ return aipva; };
		inline BurstBoss*			getBurstBoss()		{ return burst; };
		inline Ogre::Vector3		getCubeSize()		{ return cubeSize; };
		inline RuleManager*			getRuleManager()	{ return ruleManager; };
		inline SubScore*			getSubScore()		{ return subScore; };

		void groupCollisionObjects();	/// update "___Collision" arrays
		void prepareCollisionObjects();	/// cache <collision parameters> of all objects

		inline std::vector<GameplayObject*>& getCauseCollision() { return causeCollision; };
		inline std::vector<GameplayObject*>& getAcceptCollision() { return acceptCollision; };
		inline std::vector<GameplayObject*>& getHybridCollision() { return hybridCollision; };

		inline void insertBall(GameplayObject* obj) { balls[obj] = true; };
		inline void removeBall(GameplayObject* obj) { balls.erase(obj); };
		inline const std::map<GameplayObject*, bool>& getBalls() { return balls; };

		/// create a 'BillboardSet' attached to this gameplay's root 'SceneNode'
		/// =>	material name
		///		default billboard size x/y
		///		material slices (x divisions)
		///		material stacks (y divisions)
		///		billboards pool size
		///		accurate facing?
		Ogre::BillboardSet* newBBSet(std::string, double, double, int = 1, int = 1, int = 20, bool = false);
		void destroyBBSet(Ogre::BillboardSet*);
		/// =>	BBSet ID
		///		...
		Ogre::BillboardSet* setBBSet(GameplayBase::BBSet, std::string, double, double, int = 1, int = 1, int = 20, bool = false);
		Ogre::BillboardSet* getBBSet(GameplayBase::BBSet);

		static const std::string BBSET_NAME; /// base name for 'BillboardSet's

		friend class GameplayObjectsIterator;
	};



	// class to visit all gameplay objects of this base
	class GameplayObjectsIterator {

	private:

		std::map< std::deque<GameplayObject*> *, bool>* map;				/// current base being visited
		std::map< std::deque<GameplayObject*> *, bool>::iterator iterator;	/// iterator to current base's gameplay object deques
		int currentDequeIndex;					/// current deque index (iterator)
		GameplayObject** currentGameplayObject;	/// current object being referenced

	public:

		/* begin new loop in 'GameplayObject's			*/;
		/* => base	= GameplayBase pointer				*/;
		/*	  obj	= visitor pointer					*/;
		/* <= is there a GameplayObject to begin with?	*/;
		bool begin(GameplayBase*, GameplayObject*&);

		/* update visitor to point to next 'GameplayObject'	*/;
		/* <= have all GameplayObjects already been visited?*/;
		bool next();

		void removeGameplayObject();
	};

}

#endif