#include "StdAfx.h"
#include "gui\SubPaused.h"

namespace P3D {

	SubPaused::SubPaused(std::string callerName) : SubMenuMG(callerName + "SubPaused\\", "Paused.xml"), FANLib::MCBSystem(_last) {
	};

	void SubPaused::setActive() {
		this->SubMenuMG::setActive();
		this->FANLib::CallBackSystem::callBackSafe(); /// I have faded in
	};

	void SubPaused::setInactive() {
		this->SubMenuMG::setInactive();
		this->FANLib::CallBackSystem::callBackSafe(); /// I have faded out
	};

	void SubPaused::widgetClicked(MyGUI::Widget* widget) {

		// if you want more buttons in this menu, check "SubLose.cpp"
		this->FANLib::MCBSystem::callBackSafe(RETRY);
		close();
	}
}