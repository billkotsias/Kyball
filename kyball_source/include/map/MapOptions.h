// -------------------------------
// Options returned by Map to Game (cosmos selection)
// -------------------------------

#pragma once
#ifndef MapOptions_H
#define MapOptions_H

#include <string>
#include <map>

namespace P3D {

	class MapOptions {

	public:

		enum MainOption {
			UNSET,		/// unset
			START_GAME,	/// continue to game
			GOTO_MENU,	/// go back to menu
			QUIT_APP,	/// quit application

			SHOW_END_CREDITS,
		};

		enum CosmosID {
			ARCTIC,
			FOREST,
			BATCAVE,
			REEF,
			HALLOWEEN,
			SAVANNA,
			DESERT,
			BEACH,

			UNDEFINED,	/// unset (& last)
		};

#ifdef _DEMO
		enum Chapter {
			FIRST,
			SECOND,
			//THIRD,
			_last_Chapter,
			//FOURTH,

		};
#else
		enum Chapter {
			FIRST,
			SECOND,
			THIRD,
			_last_Chapter,
			//FOURTH,

		};
#endif

	private:

		MainOption mainOption;	/// final option made by user
		CosmosID cosmosID;		/// Cosmos selected by user
		int level;				/// level number selected

		static std::vector< std::vector<CosmosID> > chapterCosmoi;
		static const std::map<CosmosID, std::string> initCosmosIDToStr();
		static const std::map<CosmosID, std::string> initCosmosIDToName();

	public:

		static const std::map<CosmosID, std::string> cosmosIDToStr;
		static const std::map<CosmosID, std::string> cosmosIDToName();
		static const std::vector< std::vector<CosmosID> >& getChapterCosmoi();
		static std::string getCosmosStrFromID(CosmosID cosmosID) { return cosmosIDToStr.find(cosmosID)->second; };

		MapOptions();

		MainOption getMainOption()	{ return mainOption; };
		CosmosID getCosmosID()		{ return cosmosID; };
		std::string getCosmosStr()	{ return getCosmosStrFromID(this->cosmosID); };
		int getLevel()				{ return level; };

		void reset(); /// clear selections
	
		friend class Map;
};
}

#endif