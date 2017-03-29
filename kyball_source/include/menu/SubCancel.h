// -------------------
// SubCancel - submenu
// -------------------

#pragma once
#ifndef SubCancel_H
#define SubCancel_H

#include "SubMenuMG.h"

namespace P3D {

	class SubCancel : public SubMenuMG {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);

	public:

		const std::string caption;

		std::string text;

		SubCancel(std::string callerName);

		virtual void open();
	};

}

#endif
