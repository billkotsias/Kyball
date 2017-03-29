#include "StdAfx.h"
#include "menu\SubOKCancel.h"

namespace P3D {

	SubOKCancel::SubOKCancel() : SubMenuMG("SubOKCancel\\", "OKCancel.xml"),
		caption(name + "CAPTION"), ok(name + "OK"), cancel(name + "CANCEL") {
	}

	void SubOKCancel::open() {
		this->SubMenuMG::open();

		((MyGUI::Edit*)(MyGUI::Gui::getInstancePtr()->findWidgetT(caption)))->setCaption(text);
	}

	void SubOKCancel::widgetClicked(MyGUI::Widget* widget) {

		const std::string& name = widget->getName();

		if (name == ok) {

			value = true;

		} else if (name == cancel) {

			value = false;
		}

		close();
	}

}