// ---------------------
// SubOKCancel - submenu
// ---------------------

#pragma once
#ifndef SubOKCancel_H
#define SubOKCancel_H

#include "SubMenuMG.h"

namespace P3D {

	class SubOKCancel : public SubMenuMG {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);

	public:

		const std::string caption;
		const std::string ok;
		const std::string cancel;

		std::string text;
		bool value; /// read this to see what the user selected

		SubOKCancel();

		virtual void open();
	};

}

#endif
