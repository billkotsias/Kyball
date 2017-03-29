// -------------------
// SubMap - submenu
// -------------------

#pragma once
#ifndef SubMap_H
#define SubMap_H

#include "menu\SubMenuMG.h"

namespace P3D {

	class SubMap : public SubMenuMG {

	public:

		SubMap(std::string callerName);

		virtual void open();

		const std::string edit;
		MyGUI::Edit* wEdit;
	};

}

#endif
