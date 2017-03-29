#include "StdAfx.h"
#include "menu\SubOptions.h"

#include "gui\Widget.h"
#include "menu\PlayerBoss.h"
#include "menu\Player.h"

#include "P3DApp.h"
//#include "P3DWindow.h"
#include "P3DConfig.h"

#include <iostream>
#include <fstream>

namespace P3D {

	SubOptions::SubOptions() : SubMenuMG("SubOptions\\", "options.xml"), /*showGfx(false),*/
		sound(name + "SOUND"), music(name + "MUSIC"), ambience(name + "AMBIENCE"), quality(name + "QUALITY"), back(name + "BACK"), changes(name + "CHANGES") {
	}

	void SubOptions::open() {
		this->SubMenuMG::open();

		load();
		save(); /// get a job...

		/// set initial slider positions
		unsigned int amb = Player::AMBIENCE_VOL_DEF;
		unsigned int snd = Player::SFX_VOL_DEF;
		unsigned int mus = Player::MUSIC_VOL_DEF;
		Player* player = playerBoss->getActive();
		if (player) {
			amb = player->ambienceVolume;
			snd = player->sfxVolume;
			mus = player->musicVolume;
		}
		((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(sound)))->setScrollPosition(snd);
		((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(ambience)))->setScrollPosition(amb);
		((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(music)))->setScrollPosition(mus);
		((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(quality)))->setScrollPosition(P3DConfig::gameQuality);
		((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(changes)))->setVisible(false);
	}

	void SubOptions::widgetClicked(MyGUI::Widget* widget) {

		const std::string name = widget->getName();

		if (name == back) {

			close();

		} /*else if (name == gfx) {

			showGfx = !showGfx;
			if (save()) showGfx = !showGfx; /// can't save !!!
			((MyGUI::Button*)widget)->setStateCheck(showGfx);

		}*/
	}

	void SubOptions::widgetScroll(MyGUI::VScroll* widget, size_t position) {

		Player* player = playerBoss->getActive();
		if (!player) return;

		const std::string name = widget->getName();

		if (name == sound) {
			player->sfxVolume = position;
			player->setGameVolumes();

		} else if (name == music) {
			player->musicVolume = position;
			player->setGameVolumes();

		} else if (name == ambience) {
			player->ambienceVolume = position;
			player->setGameVolumes();

		} else if (name == quality) {
			P3DConfig::gameQuality = (GameQuality::Option)position;
			((MyGUI::HScroll*)(MyGUI::Gui::getInstancePtr()->findWidgetT(changes)))->setVisible(true);
			save();
		}
	}

	bool SubOptions::save() {

		/// - create file
		std::fstream file( P3DApp::OPTIONS_FILE.c_str(), std::fstream::binary|std::fstream::out);
		if (file.fail() || !file.is_open()) return true; /// error occured

		file.write(reinterpret_cast<char*>(&P3DConfig::gameQuality), sizeof(P3DConfig::gameQuality));

		// BEWARE OF THE DATA ORDER!
		//file.write(reinterpret_cast<char*>(&showGfx), sizeof(showGfx));
		//unsigned int data;
		//data = P3DWindow::getWindow()->getWidth();
		//file.write(reinterpret_cast<char*>(&data), sizeof(data));
		//data = P3DWindow::getWindow()->getHeight();
		//file.write(reinterpret_cast<char*>(&data), sizeof(data));
		//data = P3DWindow::getWindow()->getColourDepth();
		//file.write(reinterpret_cast<char*>(&data), sizeof(data));
		//data = P3DWindow::getWindow()->getFSAA();
		//file.write(reinterpret_cast<char*>(&data), sizeof(data));

		file.close();
		return false;
	}

	bool SubOptions::load()
	{
		//MyGUI::Button* wGfx = ((MyGUI::Button*)(MyGUI::Gui::getInstancePtr()->findWidgetT(gfx)));
		//wGfx->setStateCheck(showGfx);

		/// - read file
		std::fstream file( P3DApp::OPTIONS_FILE.c_str(), std::fstream::binary|std::fstream::in);
		if (file.fail() || !file.is_open()) return true; /// error occured

		file.read(reinterpret_cast<char*>(&P3DConfig::gameQuality), sizeof(P3DConfig::gameQuality));
		//file.read(reinterpret_cast<char*>(&showGfx), sizeof(showGfx));
		//wGfx->setStateCheck(showGfx);

		file.close();
		return false;
	}
}