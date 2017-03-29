/////////////
// OverlayBox
/////////////

#pragma once
#ifndef LevelShow_H
#define LevelShow_H

#include "tween\TPlayed.h"
#include "Queue.h"
#include <gui\Widget.h>

#include <vector>

namespace P3D {

	class WiText;

	class LevelShow : public TPlayed {

	protected:

		std::vector<WiText*> level;
		WiText* num;

		WiText* createWiText(const char* const font, const char* const capt, Widget::AlignX, double siz);
		void setAnim(WiText*, int, double);

		Queue* myQueue;

	public:

		static const double SIZE_START;
		static const double SIZE_END;
		static const double SIZE_PLUS; /// additional percentage in size for level number

		LevelShow();
		virtual ~LevelShow();

		void open(void*); /// open by 'Queue' callback
		void open();

		void setText(const std::string&);

		// overrides
		virtual void playlistFinished(TPlaylist*);
	};
}

#endif