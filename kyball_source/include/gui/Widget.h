/////////
// Widget - a simple MyGUI::Widget* wrapper
/////////

#pragma once
#ifndef Widget_H
#define Widget_H

#include <string>

namespace P3D {

	class Widget {

	public:

		enum AlignX {
			LEFT,
			HCENTER,
			RIGHT,
		};

		enum AlignY {
			TOP,
			VCENTER,
			BOTTOM,
		};

	protected:

		Ogre::Vector2 screen;	/// virtual 16:9 screen
		Ogre::Vector2 origin;	/// true offset to center of screen

		/// MyGUI
		MyGUI::Widget* widget;

		Ogre::Vector2 size;
		Ogre::Vector2 position;
		//float alpha; /// overcome MyGUI's stupidity

		Ogre::Vector2 align;
		AlignX alignX;
		AlignY alignY;

	public:

		Widget(MyGUI::Widget*);
		Widget(MyGUI::Widget*, const Ogre::Vector2& siz, AlignX, AlignY, const Ogre::Vector2& pos);
		virtual ~Widget();

		MyGUI::Widget* getWidget() { return widget; };

		void setAlign(AlignX, AlignY);

		const Ogre::Vector2& getSize() const { return size; };
		void setSize(const Ogre::Vector2&);

		const Ogre::Vector2& getPosition() const { return position; };
		void setPosition(const Ogre::Vector2&);
	};
}

#endif