// --------------------
// SubOptions - submenu
// --------------------

#pragma once
#ifndef SubOptions_H
#define SubOptions_H

#include "SubMenuMG.h"

namespace P3D {

	class PlayerBoss;

	class SubOptions : public SubMenuMG {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);
		virtual void widgetScroll(MyGUI::VScroll*, size_t);

	public:

		const std::string sound;
		const std::string music;
		const std::string ambience;
		const std::string quality;
		const std::string changes;
		const std::string back;

		SubOptions();

		std::string text;
		PlayerBoss* playerBoss;

		//bool showGfx; // obsolete!

		virtual void open();

		bool save();
		bool load();
	};

}

#endif
