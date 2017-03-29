#include "StdAfx.h"
#include "menu\SubChangePlayer.h"

#include "menu\SubOKCancel.h"
#include "menu\SubCreate.h"
#include "menu\PlayerBoss.h"
#include "menu\Player.h"

#include <core\Utils.h>
#include "P3DScripts.h"
#include <fsl\FSLInstance.h>
#include <fsl\FSLClassDefinition.h>

namespace P3D {

	SubChangePlayer::SubChangePlayer() : SubMenuMG("SubChangePlayer\\", "players2.xml"), subOKCancel(0),
		back(name + "BACK"), _delete(name + "DELETE"), create(name + "CREATE"), player(name + "PLAYER") {
	}

	SubChangePlayer::~SubChangePlayer() {
		delete subOKCancel;
	}

	void SubChangePlayer::open() {
		this->SubMenuMG::open();
		update();

		subOKCancel = new SubOKCancel();
		/// <subCreate> is passed-in!
	};

	void SubChangePlayer::close() {
		this->SubMenuMG::close();
		delete subOKCancel;
		subOKCancel = 0;
	}

	void SubChangePlayer::widgetClicked(MyGUI::Widget* widget)
	{
		static const std::string areYouSure = P3DScripts::p3dVars->getRoot()->getCString("areYouSure");
		const std::string& name = widget->getName();

		if (name == back) {

			close();

		} else if (name == _delete) {

			Player* player = playerBoss->getActive();
			if (!player) return;
			setInactive();
			subOKCancel->setCallBack(this, &SubChangePlayer::subOKCancelClosed);
			subOKCancel->text = areYouSure + " '#ff0000" + player->name + "#ffffff'?";
			subOKCancel->open();

		} else if (name == create) {

			createNewPlayer();

		} else {

			int playerNum;
			FANLib::Utils::fromString(playerNum, name.substr(name.size() - 1, 1));
			if ( playerBoss->playerExists(playerNum) ) {
				playerBoss->setActive(playerNum);
				update();
			} else {
				createNewPlayer();
			}
		}
	}

	void SubChangePlayer::createNewPlayer()
	{
		setInactive();
		subCreate->setCallBack(this, &SubChangePlayer::subCreateClosed);
		subCreate->open();
	}

	void SubChangePlayer::subCreateClosed(void*) {
		setActive();
		update();
		subCreate->unsetCallBack();
	}

	void SubChangePlayer::subOKCancelClosed(void*) {
		setActive();
		if (subOKCancel->value) {
			playerBoss->erase();
		}
		update();

		subOKCancel->unsetCallBack();
	}

	void SubChangePlayer::update() {

		static const std::string empty = std::string("#a0a0a0") + P3DScripts::p3dVars->getRoot()->getCString("empty");
		static const std::string nonempty = "#a0b0a0";
		static const std::string selected = "#a0ffa0";

		const std::deque<Player*>& list = playerBoss->getList();
		Player* active = playerBoss->getActive();

		for (unsigned int i = 0; i < list.size(); ++i) {
			Player* player = list.at(i);
			if (active == player) {
				setPlayerCaption(i, selected + player->name);
			} else {
				setPlayerCaption(i, nonempty + player->name);
			}
		}

		for (unsigned int i = list.size(); i < PlayerBoss::MAX_PLAYERS; ++i) {
			setPlayerCaption(i, empty);
		}

		checkCreate(); /// enable or disable
	}

	void SubChangePlayer::setPlayerCaption(unsigned int i, const std::string& name) {
		MyGUI::Widget* widget = MyGUI::Gui::getInstancePtr()->findWidgetT(player + FANLib::Utils::toString(i));
		widget->setCaption(name);
	}

	void SubChangePlayer::checkCreate() {

		const std::deque<Player*>& list = playerBoss->getList();
		MyGUI::Widget* widget = MyGUI::Gui::getInstancePtr()->findWidgetT(create);

		if (list.size() == PlayerBoss::MAX_PLAYERS) {
			widget->setEnabled(false);
		} else {
			widget->setEnabled(true);
		}
	}
}