/////////////
// OverlayBox
/////////////

#pragma once
#ifndef OverlayBox_H
#define OverlayBox_H

#include "tween\TPlayed.h"

#include <string>

namespace Ogre {

	class BorderPanelOverlayElement;

}

namespace P3D {

	class OverlayBox : public TPlayed {

	private:

		bool cloned;

	protected:

		Ogre::Vector2 screen;

		/// ogreBox
		Ogre::Overlay* ogreBox;
		Ogre::BorderPanelOverlayElement* ogreBoxCorners;
		Ogre::BorderPanelOverlayElement* ogreBoxEdgesTB;
		Ogre::BorderPanelOverlayElement* ogreBoxEdgesLR;

		/// MyGUI
		MyGUI::VectorWidgetPtr widgets;
		MyGUI::Edit* shadowText;
		MyGUI::Edit* editText;
		int fontHeight;

		Ogre::Vector2 size;
		Ogre::Vector2 scale;
		Ogre::Vector2 position;
		Ogre::Vector2 borderSize;

	public:

		static const double MYGUI_BORDERPANEL_RATIO;

		typedef std::map<std::string, Ogre::Vector2> BorderSizesMap;
		static BorderSizesMap BorderSizes;

		/// if 'clonePrefix' is set, the new box will be cloned from corresponding 'name' template
		/// Clone's name will be 'clonePrefix/name'
		OverlayBox(const std::string& name, std::string clonePrefix = "");
		virtual ~OverlayBox();

		virtual void open() = 0;
		virtual void close() = 0;

		bool capsOnly; /// allow only capital-letters

		virtual void setText(const std::string&);

		const int& getFontHeight() const { return fontHeight; };
		void setFontHeight(const int&);

		const Ogre::Vector2& getSize() const { return size; };
		void setSize(const Ogre::Vector2&);

		const Ogre::Vector2& getScale() const { return scale; };
		void setScale(const Ogre::Vector2&);

		const Ogre::Vector2& getPosition() const { return position; };
		void setPosition(const Ogre::Vector2&);

		const Ogre::Vector2& getBorderSize() const { return borderSize; };
		void setBorderSize(const Ogre::Vector2&);

		const std::string& getName() const { return ogreBox->getName(); };
	};
}

#endif