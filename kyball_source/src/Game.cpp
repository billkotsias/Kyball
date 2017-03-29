#include "StdAfx.h"
#include "game\Game.h"

#include "map\Map.h"
#include "map\MapOptions.h"
#include "menu\SubMainOptions.h"
#include "game\CosmosCreator.h"
#include "game\Gameplay.h"
#include "game\GameplayBase.h"

#include "game\Camera.h"
#include "sound\SoundBoss.h"

#include <OgreResourceGroupManager.h>

namespace P3D {

	class GameplayBase;

	Game::Game() {
		cosmosCreator = new CosmosCreator();
		gameplay = new Gameplay();
	}

	Game::~Game() {
		delete gameplay;
		delete cosmosCreator;
	}

	// adventure mode
	bool Game::adventure(Player* player) {
		bool quitApp = false;
		bool endGame = false;
		showEndCredits = false;

		Ogre::ResourceGroupManager* resManager = Ogre::ResourceGroupManager::getSingletonPtr();

		do {
			/// show map to user and wait for it to return selected options
			map->init();
			MapOptions mapOptions = map->show(SubMainOptions::ADVENTURE, player);

			/// process map options
			/// - user doesn't intend to start game?
			const MapOptions::MainOption& mainMapOption = mapOptions.getMainOption();
			if (mainMapOption != MapOptions::START_GAME)
			{
				switch (mainMapOption)
				{
					case MapOptions::GOTO_MENU:
						quitApp = false;	/// return to main menu
						break;

					case MapOptions::QUIT_APP:
						quitApp = true;		/// quit whole app
						break;

					case MapOptions::SHOW_END_CREDITS:
						quitApp = true;		/// well, not really!
						showEndCredits = true;
						break;
				}

				break; /// break <do> loop!
			}

			map->unload(); /// make space for actual game!
			SoundBoss::getSingletonPtr()->freeAllSounds(false); /// stop and free all non-music sounds

			/// if all else fails, it surely must be 'MapOptions::START'
			/// - load default resources required by all cosmos (eg. HUD, cannon, special balls, game menus...), if not already loaded
			if (!resManager->isResourceGroupLoaded("GAMEPLAY"))
				resManager->loadResourceGroup("GAMEPLAY");

			/// - load cosmos-specific resources & build standard cosmos structure
			cosmosCreator->create(mapOptions, 1);						/// 1 = we are in <single-player> mode
			GameplayBase* base = cosmosCreator->buildGameplayBase(0);	/// 0 = build gameplayBase for 1st (and only) player
			base->player = player;
			SoundBoss::getSingletonPtr()->setCamera( base->getCamera()->getOgreCamera() ); /// if > 1 players, a <compromise> MUST be made

			/// - start game !!!
			gameplay->adventure(mapOptions, base);
			/// - gameplay has returned : either the player wants to change cosmos, or he has completed this cosmos

			switch (gameplay->getExitStatus()) {
				case Gameplay::END_APP:
					quitApp = true;
					break;
				case Gameplay::GOTO_MAP:
					quitApp = false;
					endGame = false;
					break;
				case Gameplay::GOTO_MENU:
					quitApp = false;
					endGame = true;
					break;
			}

			/// - unload last cosmos
			delete base;
			cosmosCreator->uncreate();

		} while ( !(quitApp || endGame) );

		/// clean-up before leaving game
		/// - unload default resources, if loaded
		if (resManager->isResourceGroupLoaded("GAMEPLAY")) resManager->unloadResourceGroup("GAMEPLAY");

		return quitApp;
	}

	// versus mode
	bool Game::versus(void* settings) {
		return true; /// quit app
	}

	// survival mode
	bool Game::survival(void* player) {
		return true; /// quit app
	}

} // namespace