#include "StdAfx.h"
#include "map\MapOptions.h"

#include "P3DScripts.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"
#include "fsl\FSLArray.h"

#include "core\Utils.h"

namespace P3D {

	// statically initialize maps
	const std::map<MapOptions::CosmosID, std::string> MapOptions::initCosmosIDToStr() {
		std::map<MapOptions::CosmosID, std::string> map;

		map[ARCTIC] = "arctic";
		map[FOREST] = "forest";
		map[BATCAVE] = "batcave";
		map[REEF] = "reef";
		map[HALLOWEEN] = "halloween";
		map[SAVANNA] = "savanna";
		map[DESERT] = "desert";
		map[BEACH] = "beach";

		return map;
	}
	const std::map<MapOptions::CosmosID, std::string> MapOptions::cosmosIDToStr = initCosmosIDToStr();

	//

	const std::map<MapOptions::CosmosID, std::string> MapOptions::initCosmosIDToName()
	{
		std::map<MapOptions::CosmosID, std::string> map;

		static FANLib::FSLClass* p3dVarsRoot = P3DScripts::p3dVars->getRoot();
		map[ARCTIC] = p3dVarsRoot->getCString("Arctic");
		map[FOREST] = p3dVarsRoot->getCString("Forest");
		map[BATCAVE] = p3dVarsRoot->getCString("Underground");
		map[REEF] = p3dVarsRoot->getCString("CoralReef");
		map[HALLOWEEN] = p3dVarsRoot->getCString("Halloween");
		map[SAVANNA] = p3dVarsRoot->getCString("Savanna");
		map[DESERT] = p3dVarsRoot->getCString("Desert");
		map[BEACH] = p3dVarsRoot->getCString("Tropical");

		return map;
	}
	const std::map<MapOptions::CosmosID, std::string> MapOptions::cosmosIDToName()
	{
		static const std::map<MapOptions::CosmosID, std::string> map = initCosmosIDToName();
		return map;
	}

	//

	std::vector< std::vector<MapOptions::CosmosID> > MapOptions::chapterCosmoi;
	const std::vector< std::vector<MapOptions::CosmosID> >& MapOptions::getChapterCosmoi()
	{
		if (chapterCosmoi.size()) return chapterCosmoi;

		// build now!
		FANLib::FSLArray* chapters = P3DScripts::p3dLevels->getRoot()->getArray("chapters");
		for (unsigned i = 0; i < chapters->getSize(); ++i)
		{
			std::vector<CosmosID> chapter;
			FANLib::FSLArray* cosmoi = chapters->getArray(i);
			for (unsigned j = 0; j < cosmoi->getSize(); ++j) {
				chapter.push_back( (MapOptions::CosmosID)cosmoi->getInt(j) );
			}
			chapterCosmoi.push_back( chapter ); /// a COPY is written in 'chapterCosmoi', so chapter must be FILLED BEFORE copying it!
		}

		return chapterCosmoi;
	}


	MapOptions::MapOptions() {
		reset();
	}

	void MapOptions::reset() {
		mainOption = UNSET;
		cosmosID = UNDEFINED;
		level = -1;
	}
}