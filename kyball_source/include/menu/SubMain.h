// ----------------------
// SubMain - root submenu
// ----------------------

#pragma once
#ifndef SubMain_H
#define SubMain_H

#include "SubMenuMG.h"

namespace FANLib {
	class FSLClass;
}

namespace P3D {

	class TPlaylist;
	class PlayerBoss;
	class MenuBox;
	class SubMainOptions;
	class SubChangePlayer;
	class SubCreate;
	class SubOptions;

	class SubMain : public SubMenuMG {

	protected:

		MenuBox* play;
		MenuBox* options;
		MenuBox* quit;
		MenuBox* change;
		MenuBox* credits;
#ifdef _DEMO
		MenuBox* site;
		MenuBox* steam;
#endif

		void openWidget(MenuBox*, FANLib::FSLClass*);

		void widgetClicked(void*);
		void widgetClosed(void*);

		void openWidgets(void* = 0);
		void closeWidgets();

		PlayerBoss* playerBoss;

		const std::string player;
		const std::string cloud;
		const std::string percent;
		const std::string totalScore;
		//const std::string badges;
		//const std::string badge;

	public:

		SubMain(PlayerBoss*, SubMainOptions* _returnOptions);
		virtual ~SubMain();

		virtual void open();

		void startPlay(void* = 0);

		SubMainOptions* returnOptions;
		SubChangePlayer* subChangePlayer;
		SubCreate* subCreate;
		SubOptions* subOptions;

		void close();
		void launchSite(const char* const url);

		virtual void playlistFinished(TPlaylist* list);
	};

}

#endif
