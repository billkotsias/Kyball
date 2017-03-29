#include "StdAfx.h"
#include "menu\SubMain.h"

#include "menu\SubMainOptions.h"
#include "menu\SubChangePlayer.h"
#include "menu\SubCreate.h"
#include "menu\SubOptions.h"

#include "gui\MenuBox.h"
#include "gui\SubScore.h"
#include "P3DWindow.h"

#include "menu\PlayerBoss.h"
#include "menu\Player.h"
#include "P3DScripts.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"

#include "core\Utils.h"

namespace P3D {

	SubMain::SubMain(PlayerBoss* _playBoss, SubMainOptions* _returnOptions) : SubMenuMG("SubMain\\", "main.xml"),
		player(name + "PLAYER"), /*badges(name + "BADGES"), badge(name + "BADGE"), */cloud(name + "CLOUD"), percent(name + "PERCENT"), totalScore(name + "TOTALSCORE"),
		play(0), options(0), quit(0), change(0), credits(0),
#ifdef _DEMO
		site(0), steam(0),
#endif
		returnOptions(_returnOptions) {

		playerBoss = _playBoss;

		subCreate = new SubCreate();
		subCreate->playerBoss = playerBoss;

		subChangePlayer = new SubChangePlayer();
		subChangePlayer->playerBoss = playerBoss;
		subChangePlayer->subCreate = subCreate;

		subOptions = new SubOptions();
		subOptions->playerBoss = playerBoss;
	}

	SubMain::~SubMain() {
		delete play;
		delete options;
		delete quit;
		delete change;
		delete credits;
#ifdef _DEMO
		delete site;
		delete steam;
#endif

		delete subChangePlayer;
		delete subCreate;
	}

	void SubMain::open() {
		delete play;
		delete options;
		delete quit;
		delete change;
		delete credits;
#ifdef _DEMO
		delete site;
		delete steam;
#endif

		options = new MenuBox("Overlay/MenuOptions");
		play = new MenuBox("Overlay/MenuPlay");
		change = new MenuBox("Overlay/MenuChange");
		quit = new MenuBox("Overlay/MenuQuit");
		credits = new MenuBox("Overlay/MenuCredits");
#ifdef _DEMO
		site = new MenuBox("Overlay/MenuSite");
		steam = new MenuBox("Overlay/MenuSteam");
#endif

		FANLib::FSLClassDefinition* fslmenu = P3DScripts::p3dMenu->getRoot();
		openWidget(play, fslmenu->getClass("menuPlay"));
		openWidget(options, fslmenu->getClass("menuOptions"));
		openWidget(quit, fslmenu->getClass("menuQuit"));
		openWidget(change, fslmenu->getClass("menuChange"));
		openWidget(credits, fslmenu->getClass("menuCredits"));
#ifdef _DEMO
		openWidget(site, fslmenu->getClass("menuSite"));
		openWidget(steam, fslmenu->getClass("menuSteam"));
#endif
		openWidgets();

		double _delay = quit->delay + MenuBox::DISAPPEAR_TIME; /// quit is supposedly the most delayed one
		subChangePlayer->delay = _delay;
		subOptions->delay = _delay;

#ifndef _DEPLOY
		// hack
		FANLib::FSLClassDefinition* p3dLevels = P3DScripts::p3dLevels->getRoot();
		try {
			int cosmos = p3dLevels->getInt("currentCosmos");
			int level = p3dLevels->getInt("currentLevel");
			widgetClicked((FANLib::MCBSystem*)play);
		} catch(...) {}
#endif
	}

	void SubMain::openWidget(MenuBox* menuBox, FANLib::FSLClass* fsl) {
		Ogre::Vector2 screen = P3DWindow::getSize();
		Ogre::Vector2 pos( 0.5 * screen.x * P3DWindow::getPixelRatio() + fsl->getReal("x") * screen.y, (0.5 + fsl->getReal("y")) * screen.y );
		Ogre::Vector2 size( fsl->getReal("w") * screen.y, fsl->getReal("h") * screen.y );
		double delay = fsl->getReal("d");
		menuBox->setText(fsl->getCString("t"));
		menuBox->setPosition(pos);
		menuBox->defaultSize = size;
		menuBox->delay = delay;
		menuBox->setCallBack(MenuBox::CLICKED, this, &SubMain::widgetClicked);
	}

	void SubMain::close() {
		quit->setCallBack(MenuBox::CLOSED, this, &SubMain::widgetClosed); /// the most delayed one, so the last one closing
		closeWidgets();
	}

	void SubMain::openWidgets(void*)
	{
		static std::string completedStr( P3DScripts::p3dVars->getRoot()->getCString("completed") );
		static std::string totalScoreStr( P3DScripts::p3dVars->getRoot()->getCString("totalScore") );
		
		play->open();
		options->open();
		change->open();
		quit->open();
#ifdef _DEMO
		site->open();
		site->setFontHeight( site->getFontHeight() * 0.66 ); /// hard-coded BS!
		steam->open();
		steam->setFontHeight( steam->getFontHeight() * 0.66 );
#endif
		this->SubMenuMG::open(); /// parent function

		/// NOT static, cause they are destroyed all the time!!!!!! Stupid, that!
		MyGUI::Gui* myGUI = MyGUI::Gui::getInstancePtr();
		MyGUI::StaticText* wPlayer = ((MyGUI::StaticText*)(myGUI->findWidgetT(this->player)));
		MyGUI::Widget* wPercent = ((MyGUI::Edit*)(myGUI->findWidgetT(percent)));
		MyGUI::StaticImage* wCloud = ((MyGUI::StaticImage*)(myGUI->findWidgetT(cloud)));
		MyGUI::Widget* wTotalScore = ((MyGUI::Edit*)(myGUI->findWidgetT(totalScore)));

		wCloud->setVisible(false);
		wPlayer->setVisible(false);
		wPercent->setVisible(false);
		wTotalScore->setVisible(false);

		Player* player = playerBoss->getActive(); /// <NOTE> : player now overshadows (this->)player!
		if (player)
		{
#ifndef _DEMO
			if ( player->hasFinishedGame() ) credits->open(); /// it's a secret!
#endif

			/// - player static text
			wPlayer->setCaption(player->name);
			wPlayer->setVisible(true);

			unsigned completed = player->getPercentComplete();
			unsigned totalScore = player->getTotalScore();
			if (totalScore || completed) {
				wCloud->setVisible(true);
				wPercent->setCaption( completedStr + " : " + FANLib::Utils::toString(completed) + " %" );
				wPercent->setVisible(true);
				wTotalScore->setCaption( totalScoreStr + " : " + SubScore::FormatScore( totalScore ) + "" );
				wTotalScore->setVisible(true);
			}
		}
	}

	void SubMain::closeWidgets() {
		play->close();
		options->close();
		change->close();
		quit->close();
		credits->close();
#ifdef _DEMO
		site->close();
		steam->close();
#endif

		this->SubMenuMG::close();
	}

	void SubMain::widgetClosed(void* menuBox) {
		callBack();
	}

	void SubMain::widgetClicked(void* menuBox) {

		if (menuBox == ((FANLib::MCBSystem*)play)) {

			closeWidgets();
			if (!playerBoss->getActive()) {
				/// if there are players to choose from, open <SubChangePlayer>; otherwise, open directly <SubCreate>
				if (playerBoss->getList().empty()) {
					subCreate->setCallBack(this, &SubMain::startPlay);
					subCreate->open();
				} else {
					subChangePlayer->setCallBack(this, &SubMain::startPlay);
					subChangePlayer->open();
				}
			} else {
				startPlay();
			}

		} else if (menuBox == ((FANLib::MCBSystem*)options)) {

			closeWidgets();
			subOptions->setCallBack(this, &SubMain::openWidgets);
			subOptions->open();

		} else if (menuBox == ((FANLib::MCBSystem*)quit)) {

			returnOptions->finalChoice = SubMainOptions::QUIT;
			close();

		} else if (menuBox == ((FANLib::MCBSystem*)change)) {

			closeWidgets();
			subChangePlayer->setCallBack(this, &SubMain::openWidgets);
			subChangePlayer->open();

		} else if (menuBox == ((FANLib::MCBSystem*)credits)) {

			returnOptions->finalChoice = SubMainOptions::SHOW_END_CREDITS;
			close();

#ifdef _DEMO
		} else if (menuBox == ((FANLib::MCBSystem*)site)) {

			launchSite("http://kyballgame.com");

		} else if (menuBox == ((FANLib::MCBSystem*)steam)) {

			launchSite("https://fantasian.itch.io/kyball");
#endif
		}
	}

	void SubMain::launchSite(const char* const url)
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#else
#endif
	}

	void SubMain::startPlay(void*) {
		Player* player = playerBoss->getActive();
		if (player) {
			returnOptions->finalChoice = SubMainOptions::ADVENTURE;
			returnOptions->player = player;
			close();
		} else {
			openWidgets(); /// user can't enter a name..?
		}
	}

	void SubMain::playlistFinished(TPlaylist* list) {

		this->TPlayed::playlistFinished(list);
		if (lists.size()) return;

		if (list->id == OPENING) {

			setActive();

		} else if (list->id == CLOSING) {

			destroyWidgets();
		}
	}
}