/////////
// WiText - StaticText widget
/////////

#pragma once
#ifndef WiText_H
#define WiText_H

#include <gui\Widget.h>

namespace P3D {

	class WiText : public Widget {

	protected:

		double textSize;

	public:

		WiText(MyGUI::StaticText*);
		WiText(MyGUI::StaticText*, double tSize, AlignX, AlignY, const Ogre::Vector2& pos);
		virtual ~WiText() {};

		const double & getTextSize() const { return textSize; };
		void setTextSize(const double&);
		void setTextSizeOnly(const double&);
	};
}

#endif