#include <StdAfx.h>
#include "gui\WiText.h"

#include "P3DWindow.h"
#include "MyGUI_ResourceManualFont.h"

namespace P3D {

	WiText::WiText(MyGUI::StaticText* text) : Widget(text) {

		setTextSizeOnly(text->getFontHeight() / 960.); /// 960 = editor default
	}

	WiText::WiText(MyGUI::StaticText *text, double tSize, P3D::Widget::AlignX ax, P3D::Widget::AlignY ay, const Ogre::Vector2 &pos) :
		Widget(text, Ogre::Vector2::ZERO, ax, ay, pos) {

		text->setTextAlign(MyGUI::Align::Center);
		setTextSize(tSize);
	}

	void WiText::setTextSize(const double& tSize) {
		setTextSizeOnly(tSize);
		setSize( Ogre::Vector2(((MyGUI::StaticText*)widget)->getTextSize().width / screen.x, textSize) );
	}

	void WiText::setTextSizeOnly(const double& tSize) {
		textSize = tSize;
		((MyGUI::StaticText*)widget)->setFontHeight(tSize * screen.y);
	}

}
