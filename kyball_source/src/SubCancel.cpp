#include "StdAfx.h"
#include "menu\SubCancel.h"

namespace P3D {

	SubCancel::SubCancel(std::string callerName) : SubMenuMG(callerName + "SubCancel\\", "Cancel.xml"),
		caption(std::string(name) + "CAPTION") {
	}

	void SubCancel::open() {
		this->SubMenuMG::open();

		((MyGUI::Edit*)(MyGUI::Gui::getInstancePtr()->findWidgetT(caption)))->setCaption(text);
	}

	void SubCancel::widgetClicked(MyGUI::Widget* widget) {
		close();
	}

}