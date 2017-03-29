#include "StdAfx.h"
#include "map\SubMap.h"

namespace P3D {

	SubMap::SubMap(std::string callerName) : SubMenuMG(callerName + "SubMap\\", "MapText.xml"),
		edit(std::string(name) + "EDIT") {
	}

	void SubMap::open() {
		this->SubMenuMG::open();

		wEdit = (MyGUI::Edit*)(MyGUI::Gui::getInstancePtr()->findWidgetT(edit));
	}

}