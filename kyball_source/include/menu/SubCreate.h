// -------------------
// SubCreate - submenu
// -------------------

#pragma once
#ifndef SubCreate_H
#define SubCreate_H

#include "SubMenuMG.h"

namespace P3D {

	class PlayerBoss;
	class SubCancel;

	class SubCreate : public SubMenuMG {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);

		void subCancelClosed(void*);

	public:

		const std::string ok;
		const std::string cancel;
		const std::string edit;

		SubCreate();
		~SubCreate();

		virtual void open();
		virtual void close();

		PlayerBoss* playerBoss;
		MyGUI::Edit* wEdit;
		SubCancel* subCancel;
	};

}

#endif
