#include "StdAfx.h"
#include "gui\GameMenu.h"

namespace P3D {

	GameMenu::GameMenu() : SubMenuMG("GameMenu\\", "GameMenu.xml"), FANLib::MCBSystem(_last),
		menu(std::string(name) + "MENU"), map(std::string(name) + "MAP"), quit(std::string(name) + "QUIT") {
	}

	void GameMenu::widgetClicked(MyGUI::Widget* widget) {

		const std::string& name = widget->getName();

		if (name == quit) {

			this->FANLib::MCBSystem::callBackSafe(QUIT);

		} else if (name == menu) {

			this->FANLib::MCBSystem::callBackSafe(MENU);

		} else if (name == map) {

			this->FANLib::MCBSystem::callBackSafe(MAP);

		}
	}
}