// -------------------
// SubScore - submenu
// -------------------

#pragma once
#ifndef SubScore_H
#define SubScore_H

#include "menu\SubMenuMG.h"

namespace P3D {

	class GameplayBase;

	class SubScore : public SubMenuMG {

	protected:

		std::string scoreWStr;
		MyGUI::StaticText* wScore;
		GameplayBase* base;

	public:

		static const double MATCH_TIME; /// time to match actual score
		static std::string FormatScore(unsigned score);

		double score;

		SubScore(GameplayBase* base, std::string callerName);

		void run(double);

		virtual void open();
		virtual void close();
	};

}

#endif
