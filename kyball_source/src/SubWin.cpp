#include "StdAfx.h"
#include "gui\SubWin.h"
#include "gui\SubScore.h"

#include <core\Utils.h>
#include "tween\TLinear.h"

namespace P3D {

	SubWin::SubWin(std::string callerName) : SubMenuMG(callerName + "SubWin\\", "Win.xml"), FANLib::MCBSystem(_last),
		next(std::string(name) + "NEXT"), time(std::string(name) + "TIME"), expert(std::string(name) + "EXPERT"), total(std::string(name) + "TOTAL"),
		message(std::string(name) + "MESSAGE") {
	}

	SubWin::~SubWin() {
	}

	void SubWin::open(SubWin::WinType winType) {

		static std::string totalStr;

		this->SubMenuMG::open();

		/// Level/Area message
		MyGUI::Gui* myGUI = MyGUI::Gui::getInstancePtr();
		MyGUI::StaticText* wMessage = ((MyGUI::StaticText*)(myGUI->findWidgetT(message)));
		switch (winType) {
			case LEVEL:
				wMessage->setCaption("#a0ffa0LEVEL CLEARED !");
				break;
			case COSMOS:
				wMessage->setCaption("#7fffffAREA COMPLETE !");
				break;
		}

		/// expert score?
		static const double halfPeriod = 0.3;

		totalStr = SubScore::FormatScore( totalScore );
		MyGUI::StaticText* totalCaption = (MyGUI::StaticText*)myGUI->findWidgetT(total);

		if (totalScore > expertScore) {
			totalStr = "#ffffff" + totalStr + " !";
			TPlaylist* list = registerTween(
				new TLinear<MyGUI::StaticText, float>(totalCaption, 0, halfPeriod, &MyGUI::StaticText::getAlpha, &MyGUI::StaticText::setAlpha, 5.)
				)->add( new TLinear<MyGUI::StaticText, float>(totalCaption, 1, halfPeriod, &MyGUI::StaticText::getAlpha, &MyGUI::StaticText::setAlpha));
			for (int i = 0; i < 5; ++i) {
				list->add( new TLinear<MyGUI::StaticText, float>(totalCaption, 0, halfPeriod, &MyGUI::StaticText::getAlpha, &MyGUI::StaticText::setAlpha) );
				list->add( new TLinear<MyGUI::StaticText, float>(totalCaption, 1, halfPeriod, &MyGUI::StaticText::getAlpha, &MyGUI::StaticText::setAlpha) );
			}
		}

		totalCaption->setCaption(totalStr);

		/// misc
		((MyGUI::StaticText*)(myGUI->findWidgetT(time)))->setCaption( SubScore::FormatScore(timeBonus) );
		((MyGUI::StaticText*)(myGUI->findWidgetT(expert)))->setCaption( SubScore::FormatScore(expertScore) );
	}

	void SubWin::setActive() {
		this->SubMenuMG::setActive();
		this->FANLib::MCBSystem::callBackSafe(OPEN);
	}

	void SubWin::widgetClicked(MyGUI::Widget* widget) {

		const std::string& name = widget->getName();

		if (name == next) {

			this->FANLib::MCBSystem::callBackSafe(NEXT);
			close();

		}
		/// more buttons may be added here, if player has already finished game:
		/// - "replay level"
		/// - "play next low-score level"
	}
}