#include "StdAfx.h"
#include "menu\PlayerBoss.h"

#include "menu\Player.h"

#include <iostream>
#include <fstream>

namespace P3D {

	const unsigned int PlayerBoss::MAX_PLAYERS = 5;
	const char* const PlayerBoss::FILE_PLAYERS = "players2.txt";

	PlayerBoss::PlayerBoss(std::string _path) : active(0) {

		Player::setDefaultGameVolumes(); /// set default sound volume in case there is no active player

		/// init
		path = _path;
		Player::FILE_PATH = _path;

		filePlayers = path + FILE_PLAYERS;

		/// read players' file
		static const char nl = '\n';
		static const int maxSize = 256;

		std::fstream file(filePlayers.c_str(), std::fstream::in|std::fstream::binary);

		/// - does file exist already?
		if (!file.fail() && file.is_open()) {

			char tempBuff[maxSize];

			/// -> selected player
			file.getline(tempBuff, maxSize, nl);
			std::string activeName(tempBuff);

			/// -> number of players
			unsigned char plNum;
			file.read((char*)(&plNum), sizeof plNum);

			/// -> players' names
			while(plNum--) {
				file.getline(tempBuff, maxSize, nl);
				Player* player = new Player(tempBuff);
				if (player->load()) {
					delete player; /// couldn't load!
				} else {
					players.push_back(player);
					if (activeName == tempBuff) setActive(players.size() - 1, false); /// set selected player
				}
			}

			file.close();
		}
	}

	PlayerBoss::~PlayerBoss() {
		/// - delete created players, without saving!
		for (unsigned int i = 0; i < players.size(); ++i) {
			delete players.at(i);
		}
	}

	Player* PlayerBoss::create(std::string name) {

		/// Do we have enough players already ?!
		if (players.size() >= MAX_PLAYERS) return 0;

		/// Does this name already exist ?!
		for (int i = players.size() - 1; i >= 0; --i) {
			if (players.at(i)->name == name) return 0;
		}

		/// Is this an accepted name ?!?
		if (name.empty()) return 0;
		/// TEMP HACK
		//if ( eor(name, "582a934") != "tq`1urm") return 0;		// AIRPLAY
		//if ( eor(name, "987654321") != "~yzsb{a~u") return 0;	// GAMEWORLD
		//if ( eor(name, "123456789") != "pgg|zd~l`") return 0;	// AUTHORITY

		Player* player = new Player(name);
		if (player->save()) {
			delete player;
			return 0; /// couldn't create file : is name unacceptable ?!
		}

		players.push_back(player);
		setActive(players.size() - 1, false);
		updateFile();

		return player;
	}

	std::string PlayerBoss::eor(const std::string& value, const std::string& key) {
		std::string res;

		unsigned int len = value.length();
		if (len > key.length()) len = key.length();

		for (unsigned int i = 0; i < len; ++i) {
			res.push_back( value.at(i) ^ key.at(i) );
		}
		return res;
	}

	bool PlayerBoss::playerExists(unsigned int i)
	{
		if ( i < players.size() ) return true;
		return false;
	}

	Player* PlayerBoss::setActive(unsigned int i, bool _updateFile) {
		int temp = active; /// restore if newly selected is 0
		active = i;
		if (!getActive()) active = temp;

		Player* player = getActive();
		if (player) player->setGameVolumes();

		if (_updateFile) updateFile();

		return player;
	}

	Player* PlayerBoss::getActive() {
		if (active >= 0 && active < (int)players.size()) return players.at(active);
		return 0;
	}

	Player* PlayerBoss::erase() {

		Player* player = getActive();
		if (!player) return 0;

		player->erase(); /// tell player to delete its files
		delete player;

		players.erase(players.begin() + active);
		if (!getActive()) setActive(players.size() - 1, false);
		updateFile();

		return getActive();
	}

	void PlayerBoss::updateFile() {

		static const char nl = '\n';

		std::fstream file(filePlayers.c_str(), std::fstream::out|std::fstream::binary);
		if (file.fail() || !file.is_open()) return;

		/// -> selected player
		Player* pActive = getActive();
		if (pActive) file.write((char*)(pActive->name.c_str()), pActive->name.size());
		file.write(&nl, sizeof nl);

		/// -> number of players
		unsigned char plNum = players.size();
		file.write((char*)(&plNum), sizeof plNum);

		/// -> players' names
		for (unsigned int i = 0; i < players.size(); ++i) {
			const std::string& name = players.at(i)->name;
			file.write(name.c_str(), name.size());
			file.write(&nl, sizeof nl);
		}

		file.close();
	}
}