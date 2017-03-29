#include "StdAfx.h"
#include "gui\Widget.h"

#include "P3DWindow.h"
#include "SceneBaseCreator.h"

namespace P3D {

	// constructors
	Widget::Widget(MyGUI::Widget* wi) : widget(wi)/*, alpha(widget->getAlpha())*/ {

		origin = P3DWindow::getSize() / 2;

		Ogre::Vector2 tempScreen = P3DWindow::getSize();
		tempScreen.x *= P3DWindow::getPixelRatio();

		screen.y = tempScreen.y;
		screen.x = screen.y * 4 / 3; /// <NOTE> : screen is considered to be 4:3, due to that Editor's size!

		Ogre::Vector2 _pos = (
			Ogre::Vector2(
			widget->getPosition().left + widget->getSize().width / 2,
			widget->getPosition().top + widget->getSize().height / 2)
			- origin
			) / tempScreen;
		setPosition(_pos);

		setAlign(Widget::HCENTER, Widget::VCENTER);

		Ogre::Vector2 _size = Ogre::Vector2(widget->getSize().width, widget->getSize().height) / tempScreen;
		setSize(_size);
	}

	Widget::Widget(MyGUI::Widget* wi, const Ogre::Vector2& siz, AlignX ax, AlignY ay, const Ogre::Vector2& pos) : widget(wi)/*, alpha(widget->getAlpha())*/ {

		screen.y = P3DWindow::getSize().y;
		screen.x = screen.y * 16 / 9; /// <NOTE> : screen is considered to be 16:9..!

		origin = P3DWindow::getSize() / 2;

		setPosition(pos);
		setAlign(ax, ay);
		setSize(siz);
	}

	Widget::~Widget() {
		SceneBaseCreator::getSingletonPtr()->getMyGUI()->destroyWidget(widget);
	}
	//

	void Widget::setAlign(Widget::AlignX ax, Widget::AlignY ay) {
		alignX = ax;
		alignY = ay;

		switch (alignX) {
			case LEFT:
				align.x = 0;
				break;
			case HCENTER:
				align.x = - size.x / 2;
				break;
			case RIGHT:
				align.x = - size.x;
				break;
		}

		switch (alignY) {
			case TOP:
				align.y = 0;
				break;
			case VCENTER:
				align.y = - size.y / 2;
				break;
			case BOTTOM:
				align.y = - size.y;
				break;
		}

		/// reset position
		setPosition(position);
	}

	void Widget::setPosition(const Ogre::Vector2& pos) {
		position = pos;
		widget->setPosition(
			(position.x + align.x) * screen.x + origin.x,
			(position.y + align.y) * screen.y + origin.y
			);
	}

	void Widget::setSize(const Ogre::Vector2& sz) {
		size = sz;
		widget->setSize( size.x * screen.x, size.y * screen.y);

		/// reset align & position
		setAlign(alignX, alignY);
	}
}