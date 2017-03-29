#include "StdAfx.h"
#include "gui\LevelShow.h"

#include "gui\WiText.h"
#include "SceneBaseCreator.h"
#include "tween\TLinear.h"

namespace P3D {

	const double LevelShow::SIZE_START = 0.3;
	const double LevelShow::SIZE_END = 0.13;
	const double LevelShow::SIZE_PLUS = 1.35;

	LevelShow::LevelShow() : myQueue(0) {

		num = createWiText( "LogoNums", "2", Widget::LEFT, SIZE_START * SIZE_PLUS);

		level.resize(5);
		level.at(0) = createWiText( "LogoLevel", "L", Widget::RIGHT, SIZE_END);
		level.at(1) = createWiText( "LogoLevel", "E", Widget::RIGHT, SIZE_END);
		level.at(2) = createWiText( "LogoLevel", "V", Widget::RIGHT, SIZE_END);
		level.at(3) = createWiText( "LogoLevel", "E", Widget::RIGHT, SIZE_END);
		level.at(4) = createWiText( "LogoLevel", "L", Widget::RIGHT, SIZE_END);
	}

	WiText* LevelShow::createWiText(const char* const font, const char* const capt, Widget::AlignX ax, double siz) {
		MyGUI::Gui* gui = SceneBaseCreator::getSingletonPtr()->getMyGUI();

		MyGUI::StaticText* wid = (MyGUI::StaticText*)gui->createWidgetT("StaticText", "StaticText",1,1,1,1,MyGUI::Align::Center, "Back");
		wid->setVisible(false);
		wid->setFontName(font);
		wid->setCaption(capt);
		wid->setTextColour(MyGUI::Colour::White);
		return new WiText( wid, siz, ax, Widget::BOTTOM, Ogre::Vector2::ZERO );
	}

	LevelShow::~LevelShow() {
		deleteTweens();
		delete num;
		for (int i = level.size() - 1; i >= 0; --i) {
			delete level.at(i);
		}
		level.clear();
	}

	void LevelShow::open(void* queue) {
		myQueue = (Queue*)queue;
		open();
	}

	void LevelShow::open() {

		/// <level>
		level.back()->setPosition( Ogre::Vector2(0.05, -0.1) );
		level.back()->setTextSize(SIZE_END);
		for (int i = level.size() - 2; i >= 0; --i) {
			Ogre::Vector2 pos = level.at(i+1)->getPosition();
			pos.x -= level.at(i+1)->getSize().x + 0.001;
			level.at(i)->setPosition( pos );
			level.at(i)->setTextSize(SIZE_END);
		}
		for (unsigned int i = 0; i < level.size(); ++i) {
			WiText* wi = level.at(i);
			wi->getWidget()->setVisible(true);
			wi->getWidget()->setAlpha(0.f);
			wi->setTextSize(SIZE_START);
			setAnim(wi, i, SIZE_END);
		}

		/// <num>
		num->getWidget()->setVisible(true);
		num->setPosition( Ogre::Vector2(0.15, -0.1) );
		num->getWidget()->setAlpha(0.f);
		num->setTextSize(SIZE_START * SIZE_PLUS);
		setAnim(num, level.size(), SIZE_END * SIZE_PLUS);

	}

	void LevelShow::setAnim(WiText* wi, int i, double size) {

		/// timings
		static const double fade = 0.15;
		static const double delay = 0.10;
		static const double hold = 1.0;

		TPlaylist* list;

		/// - alpha
		list = new TPlaylist();
		list->add( new TLinear<MyGUI::Widget, float>(wi->getWidget(), 1.f, fade, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha, delay * i) );
		list->add( new TLinear<MyGUI::Widget, float>(wi->getWidget(), 0.f, fade, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha, hold) );
		registerList(list);

		/// - scale
		list = new TPlaylist();
		list->add( new TLinear<WiText, double>(wi, size, fade, &WiText::getTextSize, &WiText::setTextSize, delay * i) );
		list->add( new TLinear<WiText, double>(wi, size/2., fade, &WiText::getTextSize, &WiText::setTextSize, hold) );
		registerList(list);
	}

	void LevelShow::setText(const std::string& str) {
		((MyGUI::StaticText*)num->getWidget())->setCaption(str);
	}

	void LevelShow::playlistFinished(TPlaylist* list) {
		this->TPlayed::playlistFinished(list);
		if (lists.size()) return; /// more lists pending, not finished yet!

		for (int i = level.size() - 1; i >= 0; --i) {
			level.at(i)->getWidget()->setVisible(false);
		}
		num->getWidget()->setVisible(false);

		Queue* tempQueue = myQueue;
		myQueue = 0;
		if (tempQueue) tempQueue->next();
	}
}