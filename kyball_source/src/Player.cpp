#include "StdAfx.h"
#include "menu\Player.h"

#include "sound\SoundBoss.h"

#include "P3DScripts.h"
#include "fsl\FSLArray.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"

#include "math\Math.h"

namespace P3D {

	const unsigned char Player::MAX_VOL = 255;
	const unsigned char Player::SFX_VOL_DEF = MAX_VOL;
	const unsigned char Player::MUSIC_VOL_DEF = MAX_VOL / 6;
	const unsigned char Player::AMBIENCE_VOL_DEF = MAX_VOL;
	const std::string Player::FILE_EXTENSION = ".bin";

	std::string Player::FILE_PATH;

	const std::vector<std::string> Player::badgesToStr = Player::_badgesToStr();
	const std::vector<std::string> Player::_badgesToStr() {
		std::vector<std::string> b;
		b.resize(Player::_last_Badge);
		b[TUTORIAL_BEATER] = "#aaaaaaTutorial Beater";
		return b;
	}

	Player::Player(std::string _name) : name(_name), chapter(MapOptions::FIRST),
		sfxVolume(SFX_VOL_DEF), musicVolume(MUSIC_VOL_DEF), ambienceVolume(AMBIENCE_VOL_DEF), hundredPercent(false) {

		cosmoi.resize( MapOptions::UNDEFINED ); /// create 8 empty deques
		//badges.resize( _last_Badge ); /// all badges set as "not-earned"
	}

	Player::~Player() {
		if (file.is_open()) file.close();
	}

	void Player::setGameVolumes() {
		SoundBoss* snd = SoundBoss::getSingletonPtr();
		snd->setAmbienceVolume( ((float)(ambienceVolume)) / ((float)(MAX_VOL)) );
		snd->setSoundsVolume( ((float)(sfxVolume)) / ((float)(MAX_VOL)) );
		snd->setMusicVolume( ((float)(musicVolume)) / ((float)(MAX_VOL)) );;
		save();
	}

	void Player::setDefaultGameVolumes() {
		SoundBoss* snd = SoundBoss::getSingletonPtr();
		snd->setAmbienceVolume( ((float)(AMBIENCE_VOL_DEF)) / ((float)(MAX_VOL)) );
		snd->setSoundsVolume( ((float)(SFX_VOL_DEF)) / ((float)(MAX_VOL)) );
		snd->setMusicVolume( ((float)(MUSIC_VOL_DEF)) / ((float)(MAX_VOL)) );;
	}

	unsigned int Player::getNextLevel( MapOptions::CosmosID cosmosID, unsigned currentLevel )
	{
		if ( hasFinishedGame() )
		{
			unsigned levelNum = isNotExpertAt( cosmosID, currentLevel );
			if (levelNum == -1) levelNum = isNotExpertAt( cosmosID, 0, currentLevel );
			return levelNum; /// if -1 => error, shouldn't have called me
		}
		/// default
		return cosmoi.at(cosmosID).size();
	}

	bool Player::isExpertAt(MapOptions::CosmosID cosmosID)
	{
		if ( !hasFinishedGame() ) return false;
		if ( isNotExpertAt(cosmosID, 0) == -1) return true;
		return false;
	}
	unsigned Player::isNotExpertAt(MapOptions::CosmosID cosmosID, unsigned from, unsigned to)
	{
		const std::deque<unsigned> cosmosScores = cosmoi.at( cosmosID );
		if (to == -1) to = cosmosScores.size();
		const FANLib::FSLArray* fslLevels = P3DScripts::p3dLevels->getRoot()->getArray( MapOptions::cosmosIDToStr.find( cosmosID )->second.c_str() );

		for (unsigned levelNum = from; levelNum < to; ++levelNum)
		{
			bool expert = (unsigned int)(fslLevels->getClass( levelNum )->getInt( "expert" )) <= cosmosScores.at( levelNum );
			if (!expert) return levelNum;
		}
		return -1;
	}

	bool Player::checkJustNow100Percent()
	{
		if (hundredPercent) return false; /// may be, but not JUST NOW

		const std::vector< std::vector<MapOptions::CosmosID> >& chapters = MapOptions::getChapterCosmoi();
		for (int i = chapters.size() - 1; i >= 0; --i)
		{
			const std::vector<MapOptions::CosmosID>& chapter = chapters.at(i);
			for (int j = chapter.size() - 1; j >= 0; --j)
			{
				if ( !isExpertAt( chapter.at(j) ) ) return false;
			}
		}
		hundredPercent = true;
		save();
		return true;
	}

	unsigned int Player::getTotalScore()
	{
		unsigned totalScore = 0;
		for (int i = cosmoi.size() - 1; i >= 0; --i) {
			for (int j = cosmoi.at(i).size() - 1; j >= 0; --j) {
				totalScore += cosmoi.at(i).at(j);
			}
		}
		return totalScore;
	}

	unsigned int Player::getPercentComplete()
	{
		FANLib::FSLClassDefinition* fslAllLevels = P3DScripts::p3dLevels->getRoot();

		static unsigned leastCommonMultiplier = 0;
		if (leastCommonMultiplier == 0) {
			std::vector<unsigned int> numLevels;
			numLevels.resize( cosmoi.size() );
			for (int i = cosmoi.size() - 1; i >= 0; --i) {
				std::string cosmosStr = MapOptions::getCosmosStrFromID( (MapOptions::CosmosID)i );
				FANLib::FSLArray* fslLevels = fslAllLevels->getArray(cosmosStr.c_str());
				numLevels.at(i) = fslLevels->getSize();
			}
			leastCommonMultiplier = FANLib::Math::lcm(numLevels);
			std::cout << "leastCommonMultiplier:" << leastCommonMultiplier << "\n";
		}

		unsigned int percent = 0;
		
		for (int i = cosmoi.size() - 1; i >= 0; --i) {
			std::string cosmosStr = MapOptions::getCosmosStrFromID( (MapOptions::CosmosID)i );
			FANLib::FSLArray* fslLevels = fslAllLevels->getArray(cosmosStr.c_str());

			/// - count finished levels
			std::deque<unsigned int> levelScores = cosmoi.at(i);
			unsigned int finished = levelScores.size();
			for (int j = finished - 1; j >= 0; --j) {
				if ( levelScores.at(j) >= (unsigned int)fslLevels->getClass(j)->getInt("expert") ) ++finished;
			}

			percent += finished * leastCommonMultiplier / fslLevels->getSize();
		}

		return 100. * percent / leastCommonMultiplier / cosmoi.size() / 2.;
	}

	void Player::setScore(MapOptions::CosmosID cosmosID, unsigned int level, unsigned int newScore) {
		std::deque<unsigned int>& scores = cosmoi.at(cosmosID);
		if (scores.size() <= level) scores.resize(level + 1);
		if (scores.at(level) < newScore) scores.at(level) = newScore; /// keep greatest score
	}

	//std::deque<Player::Badge> Player::earnBadges() {
	//	std::deque<Badge> newEarned;

	//	if ( !badges.at(TUTORIAL_BEATER) ) {
	//		/// finished tutorial (BEACH number of scores == BEACH number of levels)
	//		if ( cosmoi.at(MapOptions::BEACH).size() == P3DScripts::p3dLevels->getRoot()->getArray(MapOptions::getCosmosStrFromID(MapOptions::BEACH).c_str())->getSize() ) {
	//			badges.at(TUTORIAL_BEATER) = true;
	//			newEarned.push_back(TUTORIAL_BEATER);
	//		}
	//	}

	//	return newEarned;
	//}

	bool Player::save() {

		/// - create file
		file.open( fullName().c_str(), std::fstream::binary|std::fstream::out);
		if (file.fail() || !file.is_open()) return true; /// error occured

		/// <NOTE> : the ouput order must be preserved!

		/// -> <sound settings>
		out(sfxVolume);
		out(musicVolume);
		out(ambienceVolume);

		/// -> <chapter>
		out(chapter);

		/// -> <Cosmoi scores>
		unsigned char cosmoiNum = cosmoi.size();
		out(cosmoiNum); /// -> number of cosmoi to be written
		for (unsigned char i = 0 ; i < cosmoiNum; ++i) {

			unsigned char cosmosID = i;
			out(cosmosID); /// -> cosmos ID
			std::deque<unsigned int>& scores = cosmoi.at(cosmosID);

			unsigned char scoresNum = scores.size();
			out(scoresNum); /// -> number of registered scores
			for (unsigned char j = 0; j < scoresNum; ++j) {
				int score = scores.at(j);
				out(score); /// -> score
			}
		}

		std::cout << std::boolalpha << "SAVING " << hundredPercent << "\n";
		out(hundredPercent);

		/// REMOVED -> <badges>
		//for (unsigned int i = 0; i < badges.size(); ++i) {
		//	bool badge = badges.at(i);
		//	out(badge);
		//}

		/// - close file
		file.close();
		return false;
	}

	bool Player::load() {

		/// - access file
		file.open( fullName().c_str(), std::fstream::binary|std::fstream::in);
		if (file.fail() || !file.is_open()) return true; /// error occured

		/// <NOTE> : the ouput order must be preserved!

		/// -> <sound settings>
		in(sfxVolume);
		in(musicVolume);
		in(ambienceVolume);

		/// -> <chapter>
		in(chapter);

		/// -> <Cosmoi scores>
		unsigned char cosmoiNum;
		in(cosmoiNum); /// -> number of cosmoi to be read
		for (unsigned char i = 0 ; i < cosmoiNum; ++i) {

			unsigned char cosmosID;
			in(cosmosID); /// -> cosmos ID
			std::deque<unsigned int>& scores = cosmoi.at(cosmosID);

			unsigned char scoresNum;
			in(scoresNum); /// -> number of registered scores
			for (unsigned char j = 0; j < scoresNum; ++j) {
				int score;
				in(score); /// -> score
				setScore((MapOptions::CosmosID)cosmosID, j, score);
			}
		}

		in(hundredPercent);
		std::cout << std::boolalpha << "LOADED " << hundredPercent << "\n";

		/// REMOVED -> <badges>
		//for (unsigned int i = 0; i < badges.size(); ++i) {
		//	bool badge;
		//	in(badge);
		//	badges.at(i) = badge;
		//}

		/// - close file
		file.close();
		return false;
	}

	void Player::erase() {
		::remove( fullName().c_str() );
	}

	template<class Class>
	void Player::out(Class value) {
		file.write(reinterpret_cast<char*>(&value), sizeof(value));
	}

	template<class Class>
	void Player::in(Class& value) {
		file.read(reinterpret_cast<char*>(&value), sizeof(value));
	}

	std::string Player::fullName() {
		return FILE_PATH + name + FILE_EXTENSION;
	}
}