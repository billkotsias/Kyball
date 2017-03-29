#include "StdAfx.h"
#include "gui\SubLose.h"

namespace P3D {

	SubLose::SubLose(std::string callerName) : SubMenuMG(callerName + "SubLose\\", "Lose.xml"), FANLib::MCBSystem(_last),
		retryWidgetName(name + "RETRY"), skipWidgetName(name + "SKIP"), skipConfirmWidgetName(name + "SKIP_CONFIRM") {
	}

	void SubLose::setActive() {
		this->SubMenuMG::setActive();
		this->FANLib::MCBSystem::callBackSafe(OPEN);
	}

	void SubLose::open() {
		this->SubMenuMG::open();
		MyGUI::Gui::getInstancePtr()->findWidgetT( skipConfirmWidgetName )->setVisible( false );
	}

	void SubLose::widgetClicked(MyGUI::Widget* widget) {

		const std::string& name = widget->getName();

		if (name == retryWidgetName) {

			this->FANLib::MCBSystem::callBackSafe(NEXT);
			close();

		} else if (name == skipWidgetName) {

			MyGUI::Widget* skipConfirm = MyGUI::Gui::getInstancePtr()->findWidgetT( skipConfirmWidgetName );
			if ( skipConfirm->isVisible() ) {
				this->FANLib::MCBSystem::callBackSafe(SKIP);
				close();
			} else {
				skipConfirm->setVisible( true );
			}

		}
	}
}