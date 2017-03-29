// --------------------------------------------------------
// PlaySettings - choices set by user in menu, used by game
// --------------------------------------------------------
// This class is only a data structure

#pragma once
#ifndef SubMainSettings_H
#define SubMainSettings_H

namespace P3D {

	class SubMainSettings {
	public:

		// user final choice : enter game in specific game mode or quit the whole lot
		/* => 0 = quit application	*/;
		/* => 1 = adventure mode	*/;
		/* => 2 = versus mode		*/;
		/* => 3 = survival mode	... etc.	*/;
		int finalChoice;

		// pointer to corresponding settings data structure, depends on 'playMode'
		void* modeSettings;

	};

}

#endif