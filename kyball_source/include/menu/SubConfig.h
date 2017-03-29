// ---------------------
// SubConfig - submenu
// ---------------------

#pragma once
#ifndef SubConfig_H
#define SubConfig_H

#include "SubMenuMG.h"
#include "GameQuality.h"

namespace P3D {

	class SubConfig : public SubMenuMG {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);

	public:

		GameQuality::Option selected;

		const std::string low;
		const std::string medium;
		const std::string high;

		SubConfig();
	};

}

#endif
