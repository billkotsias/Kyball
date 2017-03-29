// ----------------
// SubMenuMG system - based on MyGUI
// ----------------

#pragma once
#ifndef SubMenuMG_H
#define SubMenuMG_H

#include "SubMenu.h"
#include "tween\TPlayed.h"

#include <string>

namespace P3D {

	class Widget;

	class SubMenuMG : public SubMenu, public TPlayed {

	protected:

		/// MyGUI
		virtual void widgetClicked(MyGUI::Widget*) {};
		virtual void widgetScroll(MyGUI::VScroll* widget, size_t position) {};

		std::vector<Widget*> widgets;
		virtual void setActive();
		virtual void setInactive();
		void destroyWidgets();
		MyGUI::VectorWidgetPtr _widgets;

		std::string xmlFile;	/// set this in your constructor
		std::string name;

	public:

		static const int OPENING;
		static const int CLOSING;

		SubMenuMG(std::string _name, std::string _xml);
		virtual ~SubMenuMG();

		double delay;
		double fadeTime;

		virtual void open();	/// } Override these to do your stuff, but <call them in the derived functions> :
		virtual void close();	/// } this->SubMenuMG::open/close !

		virtual void playlistFinished(TPlaylist* list);

	};

}

#endif
