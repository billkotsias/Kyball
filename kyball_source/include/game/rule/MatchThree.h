/////////////
// MatchThree - a gameplay rule
/////////////

#pragma once
#ifndef MatchThree_H
#define MatchThree_H

#include "Rule.h"
#include "game\ai\AIPVListener.h"
#include "game\event\EventListener.h"
#include "tween\TPlayed.h"

#include <logic\Enabled.h>
#include <map>

namespace P3D {

	class GameplayObject;
	class AIPos;

	class MatchThree : public Rule, public EventListener, public TPlayed, public AIPVListener {

	protected:

		Rule::Result checkResult;
		std::map<GameplayObject*, std::map<GameplayObject*, bool> > links;	/// keeps links between 'GameplayObject's
		std::map<GameplayObject*, bool> moveToCannon;						/// objects moving towards cannon but NOT linked

		void link(GameplayObject*);			/// an object requests to be linked
		bool unlink(GameplayObject*);		/// an object requests to be unlinked; => object, <= was object previously linked?
		void matchThree(GameplayObject*);	/// does this object satisfy the "match-three" condition?

		/// check whether there are objects no longer attached on a <firm> point
		/// =>	calculate "distance-effect" starting from this object
		/// <=	number of falling objects
		///		number of objects to count for score <!>
		void checkFalling(GameplayObject* = 0, unsigned int* fallingNum = 0, unsigned int* countForScore = 0);

		/// advance 'mover' objects towards cannon
		void advanceToCannon();
		void advanceToCannon(GameplayObject*, double amount); /// advance one

		inline void insertMover(GameplayObject* obj) { moveToCannon[obj] = true; };
		inline void removeMover(GameplayObject* obj) { moveToCannon.erase(obj); };

		void checkLose(AIPos*);			/// check if this 'GameplayObject' causes 'Player' to <lose>
		void checkWin();				/// check if 'Player' has <finished> this level
		void checkAnimalEmotion(bool);	/// what should be the animal's current emotional state? => if true, make "sad" instead of "anxious"

		unsigned int matches;		/// matched balls required for an explosion
		unsigned int advanceEvery;	/// shrink cube every this many shots
		unsigned int toAdvance;		/// shots left to shrinkage

		/// Cannon-relative
		FANLib::DisableID disableID;
		void disableCannon();
		void enableCannon();
		void nextCannonRound();

	public:

		static const double TOLERANCE; /// distance error-tolerance
		static const double TIME_TO_ADVANCE;

		static const unsigned int POP_SCORE;
		static const unsigned int FALL_SCORE;
		static const unsigned int MAX_FALLING_SCORE;

		static const double ANXIOUS_EXTENT;
		static const double SMILE_EXTENT;

		/// =>	'GameplayBase'
		///		matched balls required for an explosion
		MatchThree(GameplayBase* inBase, unsigned int = 3);
		virtual ~MatchThree();

		// overrides
		virtual void incomingEvent(Event*);
		virtual void animeOver(AIPVAnime*);
		virtual void playlistFinished(TPlaylist*);
		virtual Rule::Result perCycleCheck();
	};
}

#endif