#include "StdAfx.h"
#include "menu\SubCreate.h"

#include "menu\SubCancel.h"
#include "menu\PlayerBoss.h"
#include "menu\Player.h"

namespace P3D {

	SubCreate::SubCreate() : SubMenuMG("SubCreate\\", "CreatePlayer.xml"),
		ok(name + "OK"), cancel(name + "CANCEL"), edit(name + "EDIT"), subCancel(0) {
	}

	SubCreate::~SubCreate() {
		delete subCancel;
	}

	void SubCreate::open() {
		this->SubMenuMG::open();

		wEdit = (MyGUI::Edit*)(MyGUI::Gui::getInstancePtr()->findWidgetT(edit));
		wEdit->eventEditSelectAccept = MyGUI::newDelegate(this, &SubCreate::widgetClicked);
		MyGUI::InputManager::getInstance().setKeyFocusWidget(wEdit);
	}

	void SubCreate::close() {
		this->SubMenuMG::close();

		delete subCancel;
		subCancel = 0;
	}

	void SubCreate::widgetClicked(MyGUI::Widget* widget) {

		const std::string& name = widget->getName();

		if (name == cancel) {

			close();

		} else if (name == ok || name == edit) {

			std::string playerName = wEdit->getOnlyText();
			Player* player = playerBoss->create(playerName);
			if (!player) {

				setInactive();
				if (!subCancel) subCancel = new SubCancel(name);
				subCancel->setCallBack(this, &SubCreate::subCancelClosed);
				subCancel->text = std::string("#ffffffCannot create player #ffa0a0'") + playerName + "'#ffffff. Please try another name.";
				subCancel->open();

			} else {

				close();

			}

		}
	}

	void SubCreate::subCancelClosed(void*) {
		setActive();
		MyGUI::InputManager::getInstance().setKeyFocusWidget(wEdit);
	}

}