// ----------------------
// Player - player status
// ----------------------

#pragma once
#ifndef Player_H
#define Player_H

#include "map\MapOptions.h"

#include <string>
#include <deque>
#include <vector>

#include <iostream>
#include <fstream>

namespace P3D {

	class Player {

	public:

		static const std::vector<std::string> badgesToStr;
		static const std::vector<std::string> _badgesToStr();

		static const unsigned char MAX_VOL;
		static const unsigned char SFX_VOL_DEF;
		static const unsigned char MUSIC_VOL_DEF;
		static const unsigned char AMBIENCE_VOL_DEF;
		static const std::string FILE_EXTENSION;

		enum Badge {
			TUTORIAL_BEATER,

			_last_Badge
		};

		static std::string FILE_PATH;	/// common file <path> for all players

		Player(std::string _name);
		~Player();

		std::string name;				/// also corresponds to <filename>
		std::vector< std::deque<unsigned int> > cosmoi; /// scores per level per cosmos
		//std::vector<bool> badges; /// REMOVED
		bool hasFinishedGame() { return chapter == MapOptions::_last_Chapter; };

		/// <percent>
		unsigned int getPercentComplete();
		unsigned int getTotalScore();
		bool checkJustNow100Percent();
		bool is100Percent() { return hundredPercent; };

		unsigned int getNextLevel(MapOptions::CosmosID, unsigned currentLevel = 0);
		bool isExpertAt(MapOptions::CosmosID);

		/// set a <new score> for a level; will be updated only if <greater> than current
		/// => Cosmos, level number, new score
		void setScore(MapOptions::CosmosID, unsigned int, unsigned int);

		/// <settings>
		unsigned char sfxVolume;
		unsigned char musicVolume;
		unsigned char ambienceVolume;

		void setGameVolumes();
		static void setDefaultGameVolumes();

		// REMOVED - check for earned badges
		// <= newly earned badges
		//std::deque<Badge> earnBadges();

		/// <save>
		/// <= true if an error occured during saving
		bool save();

		/// <load>
		/// <= true if an error occured during loading
		bool load();

		/// <erase> corresponding file
		void erase();

		std::string fullName();

	private:

		MapOptions::Chapter chapter;
		bool hundredPercent;

		// returns 1st level in range where player is NOT an expert, else -1
		// 'to' is EXCLUSIVE, default means till end
		unsigned isNotExpertAt(MapOptions::CosmosID, unsigned from, unsigned to = -1);

		std::fstream file; /// corresponding file stream

		template<class Class>
		void out(Class value);

		template<class Class>
		void in(Class& value);

		friend class Map;
	};

}

#endif // Player_H