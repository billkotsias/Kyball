// -------------------
// GameMenu - submenu
// -------------------

#pragma once
#ifndef GameMenu_H
#define GameMenu_H

#include "menu\SubMenuMG.h"
#include <core\MCBSystem.h>

namespace P3D {

	class GameplayBase;

	class GameMenu : public SubMenuMG, public FANLib::MCBSystem {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);

	public:

		enum CallBacks {
			_none,
			MAP,
			MENU,
			QUIT,
			_last
		};

		const std::string map;
		const std::string menu;
		const std::string quit;

		GameMenu();
	};

}

#endif
