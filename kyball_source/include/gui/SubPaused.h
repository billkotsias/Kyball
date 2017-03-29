// -------------------
// SubPaused - submenu
// -------------------

#pragma once
#ifndef SubPaused_H
#define SubPaused_H

#include "menu\SubMenuMG.h"
#include <core\MCBSystem.h>

namespace P3D {

	class SubPaused : public SubMenuMG, public FANLib::MCBSystem {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);
		virtual void setActive();
		virtual void setInactive();

	public:

		enum CallBacks {
			_none,
			RETRY,
			/// more may be added
			_last
		};

		SubPaused(std::string callerName);
	};

}

#endif
