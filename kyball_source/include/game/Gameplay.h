// -----------------------
// Gameplay - here at last
// -----------------------

#pragma once
#ifndef Gameplay_H
#define Gameplay_H

#include "Queue.h"
#include "hid\HIDListener.h"

#include "map\MapOptions.h"
#include "OgreFrameListener.h"
#include "collision\CollisionParams.h"
#include "game\rule\Rule.h"

#include <logic\Enabled.h>
#include <deque>

namespace FANLib {
	class FSLArray;
}

namespace P3D {

	class GameplayBase;
	class GameplayObject;
	class Player;
	class LevelBuilder;
	class HintBox;
	class LevelShow;
	class GameMenu;
	class SubMenuMG;
	class SubWin;
	class SubLose;
	class SubPaused;

	/// Editor stuff
	class BallEditor;

	class Gameplay: public Ogre::FrameListener, public HIDListener {

	public:

		enum ExitStatus {
			GOTO_MAP,
			GOTO_MENU,
			END_APP,	/// e.g window was closed

			UNDEFINED,
		};

		static const double WIN_TIME;
		static const double LOSE_TIME;

	private:

		LevelBuilder* levelBuilder;

		FANLib::FSLArray* currentCosmosLevels;

		HintBox* hintBox;
		LevelShow* levelShow;
		GameMenu* gameMenu;
		SubPaused* subPause;
		SubWin* subWin;
		SubLose* subLose;

		Rule::Result gameplayResult;
		ExitStatus exitStatus; /// for 'Game'

		// Cannon & Camera relative
		FANLib::DisableID disableID;
		void disableAllCannons(bool = false);
		void enableAllCannons(bool = false);
		void disableAllCameras();
		void enableAllCameras();

		void openGameMenu(void* = 0);
		void closeGameMenu(void* = 0);

		// Gameplay
		bool frameRenderingQueued(const Ogre::FrameEvent&);
		Rule::Result gameplay(GameplayBase*);
		void win(GameplayBase*, double delay);
		void lose(GameplayBase*, double delay);
		void winOpen(void* = 0); /// play sound, trigger cube effect, open game menu
		void won(void* = 0);
		void lost(void* = 0);
		void skip(void* = 0);

		void quitNow(void* = 0);
		void gotoMap(void* = 0);
		void gotoMenu(void* = 0);

		double gameTime;

		bool started;
		bool paused;
		bool pausedFading;
		void togglePause();

		void pauseEvent(void*) {
			pausedFading = false;
		};

		void start(void*);

		// GUI
		void initScores();

		// internal service functions
		// - check collision between 2 objects and update 'earlier' and 'futureCollisions'
		//void checkCollision(GameplayObject*, GameplayObject*, CollisionParams&, std::map<double, CollisionParams>& );
		void checkCollision(GameplayObject*, GameplayObject*, std::map<CollisionParams, bool>& );

		/// Editor stuff
		BallEditor* bEdit;

		// overrides
		virtual void onKeyEvent();
		virtual void onMouseChange();

		//
		// debug
		///OIS Input devices
		unsigned int currentLevel;

		void debugMisc();
		Rule::Result debugMoveCamera();
		void debugUpdateStats();

	public:

		// constructor
		Gameplay();
		~Gameplay();

		std::deque<GameplayBase*> gameplayBases;

		// start gameplay in adventure mode
		/// =>	mapOptions	= we are interested in the currently selected cosmos
		///		base		= only 1 GameplayBase is needed in 'adventure' mode
		void adventure(MapOptions, GameplayBase*);

		inline ExitStatus getExitStatus() { return exitStatus; };
	};

}

#endif