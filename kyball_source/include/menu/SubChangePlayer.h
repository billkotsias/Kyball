// -------------------------
// SubChangePlayer - submenu
// -------------------------

#pragma once
#ifndef SubChangePlayer_H
#define SubChangePlayer_H

#include "SubMenuMG.h"
#include <string>

namespace P3D {

	class SubCreate;
	class SubOKCancel;
	class PlayerBoss;

	class SubChangePlayer : public SubMenuMG {

	protected:

		void setPlayerCaption(unsigned int i, const std::string&);

		virtual void widgetClicked(MyGUI::Widget*);
		void subCreateClosed(void*);
		void subOKCancelClosed(void*);
		void createNewPlayer();

	public:

		const std::string back;
		const std::string create;
		const std::string _delete;
		const std::string player;

		SubChangePlayer();
		~SubChangePlayer();

		SubCreate* subCreate;
		SubOKCancel* subOKCancel;
		PlayerBoss* playerBoss;

		virtual void open();
		virtual void close();

		virtual void playlistFinished(TPlaylist* list) {
			this->SubMenuMG::playlistFinished(list);
			if (list->id == OPENING) update();
		};

		void update();
		void checkCreate();
	};

}

#endif
