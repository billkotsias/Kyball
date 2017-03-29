// --------------------------------------------------------------------
// SubMainOptions - options set by user in "main submenu", used by game
// --------------------------------------------------------------------
// This class is only a data structure

#pragma once
#ifndef SubMainOptions_H
#define SubMainOptions_H

namespace P3D {

	class Player;

	class SubMainOptions {

	public:

		enum FinalChoice {
			QUIT,		// quit application

			ADVENTURE,	// adventure mode
			VERSUS,		// versus mode
			SURVIVAL,	// survival mode

			SHOW_END_CREDITS,

						// ... etc
		};

		FinalChoice finalChoice;	/* user final choice : enter game in specific game mode or quit the whole lot */;
		Player* player;				/* pointer to corresponding settings data structure, depends on 'playMode' */;

	};

}

#endif