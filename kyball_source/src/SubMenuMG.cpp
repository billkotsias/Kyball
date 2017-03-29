#include "StdAfx.h"
#include "menu\SubMenuMG.h"

#include "gui\WiText.h"
#include "tween\TLinear.h"

namespace P3D {

	const int SubMenuMG::OPENING = 1;
	const int SubMenuMG::CLOSING = 2;

	SubMenuMG::SubMenuMG(std::string _name, std::string _xml) : delay(0.), fadeTime(0.3), name(_name), xmlFile(_xml) {
	}

	SubMenuMG::~SubMenuMG() {
		deleteTweens();
		destroyWidgets();
	}

	void SubMenuMG::destroyWidgets() {
		for (int i = widgets.size() - 1; i >= 0; --i) {
			delete widgets.at(i);
		}
		widgets.clear();
		//MyGUI::LayoutManager::getInstance().unloadLayout(_widgets); // this crashes, cause widgets are destroyed ("unloaded") above
	}

	void SubMenuMG::open() {

		_widgets = MyGUI::LayoutManager::getInstance().loadLayout(xmlFile, name);
		for (unsigned int i = 0; i < _widgets.size(); ++i) {
			MyGUI::Widget* mWidget = _widgets.at(i);
			if (mWidget->getTypeName() == "StaticText" || mWidget->getTypeName() == "Edit" ||
				mWidget->getTypeName() == "Button" || mWidget->getTypeName() == "MenuItem" || mWidget->getTypeName() == "ComboBox") {
				widgets.push_back( new WiText((MyGUI::StaticText*)mWidget) );
			} else {
				widgets.push_back( new Widget(mWidget) );
			}
		}

		for (unsigned int i = 0; i < widgets.size(); ++i) {
			Widget* widget = widgets.at(i);
			MyGUI::Widget* mWidget = widget->getWidget();
			mWidget->setVisible(true);
			float maxAlpha = mWidget->getAlpha(); /// maximum alpha is the one defined in the xml
			mWidget->setAlpha(0.f);
			TPlaylist* list = registerTween(new TLinear<MyGUI::Widget, float>(
				mWidget, maxAlpha, fadeTime, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha, delay)
			);
			list->id = OPENING;
		}
	}

	void SubMenuMG::setInactive() {
		for (unsigned int i = 0; i < widgets.size(); ++i) {
			MyGUI::Widget* widget = widgets.at(i)->getWidget();
			widget->setEnabled(false);
		}
	}

	void SubMenuMG::setActive() {
		for (unsigned int i = 0; i < widgets.size(); ++i) {
			MyGUI::Widget* widget = widgets.at(i)->getWidget();
			if (widget->getName().empty()) {
				widget->setEnabled(false);
			} else {
				if (widget->getTypeName() == "HScroll") {
					((MyGUI::HScroll*)widget)->eventScrollChangePosition = MyGUI::newDelegate(this, &SubMenuMG::widgetScroll);
				} else {
					widget->eventMouseButtonClick = MyGUI::newDelegate(this, &SubMenuMG::widgetClicked);
				}
				widget->setEnabled(true);
			}
		}
		MyGUI::EnumeratorWidgetPtr it = MyGUI::Gui::getInstancePtr()->getEnumerator();
		while (it.next()) {
			MyGUI::WidgetPtr wi = it.current();
			//std::cout << "WI:" << wi->getName() << " = " << wi->isEnabled() << "\n";
		}
		//std::cout << "\n";
	}

	void SubMenuMG::close() {

		setInactive();

		for (unsigned int i = 0; i < widgets.size(); ++i) {
			Widget* widget = widgets.at(i);
			TPlaylist* list = registerTween(new TLinear<MyGUI::Widget, float>(
				widget->getWidget(), 0.f, fadeTime, &MyGUI::Widget::getAlpha, &MyGUI::Widget::setAlpha)
			);
			list->id = CLOSING;
		}
	}

	void SubMenuMG::playlistFinished(TPlaylist* list) {

		this->TPlayed::playlistFinished(list);

		if (list->id == OPENING) {

			if (existsID(OPENING)) return;
			setActive();

		} else if (list->id == CLOSING) {

			if (existsID(CLOSING)) return;
			deleteTweens(); /// there may by other tweens! (SubWin crashes!)
			destroyWidgets();
			callBackSafe();
		}
	}

}