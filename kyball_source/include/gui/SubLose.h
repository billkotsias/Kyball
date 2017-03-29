// -------------------
// SubLose - submenu
// -------------------

#pragma once
#ifndef SubLose_H
#define SubLose_H

#include "menu\SubMenuMG.h"
#include <core\MCBSystem.h>

namespace P3D {

	class SubLose : public SubMenuMG, public FANLib::MCBSystem {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);
		virtual void setActive();

	public:

		enum CallBacks {
			_none,
			OPEN,
			NEXT,
			SKIP,
			/// more may be added
			_last
		};

		const std::string retryWidgetName;
		const std::string skipWidgetName;
		const std::string skipConfirmWidgetName;

		SubLose(std::string callerName);

		virtual void open();

	};

}

#endif