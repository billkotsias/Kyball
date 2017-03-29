#include "StdAfx.h"
#include "menu\SubConfig.h"

namespace P3D {

	SubConfig::SubConfig() : SubMenuMG("SubConfig\\", "Config.xml"),
		low(std::string(name) + "LOW"), medium(std::string(name) + "MEDIUM"), high(std::string(name) + "HIGH") {
	}

	void SubConfig::widgetClicked(MyGUI::Widget* widget) {
		const std::string name = widget->getName();

		if (name == low) {
			selected = GameQuality::LOW;
			close();
		} else if (name == medium) {
			selected = GameQuality::MEDIUM;
			close();
		} else if (name == high) {
			selected = GameQuality::HIGH;
			close();
		}
	}
}