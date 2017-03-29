// -------------------
// SubBlack - submenu
// -------------------

#pragma once
#ifndef SubBlack_H
#define SubBlack_H

#include "menu\SubMenuMG.h"

namespace P3D {

	class SubBlack : public SubMenuMG {

	public:

		SubBlack(std::string callerName, std::string xml = "Black.xml") : SubMenuMG(callerName, xml) {
		};

		virtual void setActive() {
			callBackSafe(); /// this means : we want to fade out the screen to start loading or something, so we want a callback when it's faded in!
		};

	};

}

#endif
