// ----------------
// Game - main game
// ----------------

#pragma once
#ifndef GAME_H
#define GAME_H

namespace P3D {

	class Player;
	class Map;
	class CosmosCreator;
	class Gameplay;

	class Game {

	private:

		CosmosCreator* cosmosCreator;
		Gameplay* gameplay;

	public:

		// constructor/destructor
		Game();
		~Game();

		Map* map; /// passed-in!

		// game modes
		// <= all return whether there was a request within the game to end immediately the application
		bool adventure(Player*);	/* adventure mode */;
		bool versus(void*);			/// TODO : change to VersusSettings*
		bool survival(void*);		/// TODO : change to Player*

		bool showEndCredits;
	};
}

#endif