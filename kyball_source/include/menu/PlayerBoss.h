// ---------------------------
// PlayerBoss - manage players
// ---------------------------

#pragma once
#ifndef PlayerBoss_H
#define PlayerBoss_H

#include <deque>
#include <string>

namespace P3D {

	class Player;

	class PlayerBoss {

	private:

		std::string filePlayers;
		std::deque<Player*> players;
		int active; /// active/selected Player

		std::string eor(const std::string& value, const std::string& key);

	public:

		static const unsigned int MAX_PLAYERS;
		static const char* const FILE_PLAYERS;

		// constructor
		// => path of "players" file
		PlayerBoss(std::string _path);
		~PlayerBoss();

		std::string path;

		Player* create(std::string name); /// <= return pointer to new player
		Player* erase(); /// delete active player; <= return pointer to current active player

		Player* setActive(unsigned int i, bool _updateFile = true);
		Player* getActive();
		bool playerExists(unsigned int i);

		const std::deque<Player*>& getList() { return players; };

		unsigned int playersNum() { return players.size(); };

		void updateFile();
	};

}

#endif
