#include "StdAfx.h"
#include "gui\SubScore.h"

#include "game\GameplayBase.h"

#include <core\Utils.h>
#include <math.h>

namespace P3D {

	const double SubScore::MATCH_TIME = 0.5;

	SubScore::SubScore(GameplayBase* _base, std::string callerName) : SubMenuMG(callerName + "SubScore\\", "Score.xml"),
		scoreWStr(std::string(name) + "SCORE"), base(_base), wScore(0), score(0) {
	}

	void SubScore::open() {
		if (wScore) return;

		this->SubMenuMG::open();

		wScore = ((MyGUI::Edit*)(MyGUI::Gui::getInstancePtr()->findWidgetT(scoreWStr)));
		run(0.);
	}

	void SubScore::close() {
		this->SubMenuMG::close();

		wScore = 0;
	}

	void SubScore::run(double time)
	{
		if (!wScore) return;

		if (time > MATCH_TIME) time = MATCH_TIME;
		double diff = (base->score - score) * time * (1. / MATCH_TIME);
		if (diff < 0) {
			score += floor(diff);
		} else {
			score += ceil(diff);
		}

		wScore->setCaption( FormatScore( score ) );
	}

	std::string SubScore::FormatScore(unsigned score)
	{
		std::string scoreStr = FANLib::Utils::toString(score);
		int insertPosition = scoreStr.length() - 3;
		while (insertPosition > 0) {
			scoreStr.insert(insertPosition, ",");
			insertPosition-=3;
		}

		// old
		//static const char* const ZEROS = "000000000000000";
		//static const unsigned int LENGTH = 7;
		//std::string scoreStr = FANLib::Utils::toString(score);
		//int toMatch = LENGTH - scoreStr.size();
		//if (toMatch > 0) scoreStr.insert(0, ZEROS, toMatch); /// add leading zeros

		return scoreStr;
	}
}