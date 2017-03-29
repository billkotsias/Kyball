// -------------------
// SubWin - submenu
// -------------------

#pragma once
#ifndef SubWin_H
#define SubWin_H

#include "menu\SubMenuMG.h"
#include <core\MCBSystem.h>

namespace P3D {

	class SubWin : public SubMenuMG, public FANLib::MCBSystem {

	protected:

		virtual void widgetClicked(MyGUI::Widget*);
		virtual void setActive();

	public:

		enum CallBacks {
			_none,
			OPEN,
			NEXT,
			/// more may be added, as stated in .cpp file
			_last
		};

		enum WinType {
			LEVEL,
			COSMOS,
		};

		unsigned int timeBonus;
		unsigned int totalScore;
		unsigned int expertScore;

		/// captions names (differ per base)
		const std::string message;
		const std::string next;
		const std::string time;
		const std::string expert;
		const std::string total;

		SubWin(std::string callerName);
		virtual ~SubWin();

		void open(SubWin::WinType);

	};

}

#endif
