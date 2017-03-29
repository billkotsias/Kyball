// --------------
// SubMenu system
// --------------

#pragma once
#ifndef SubMenu_H
#define SubMenu_H

#include <core\CallBack.h>

namespace P3D {

	class SubMenu : public FANLib::CallBackSystem {

	public:

		virtual ~SubMenu() {};

		/// override this to do your stuff when opening
		virtual void open() = 0;

	};

}

#endif
