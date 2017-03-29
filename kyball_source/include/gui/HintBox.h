/////////////
// HintBox
/////////////

#pragma once
#ifndef HintBox_H
#define HintBox_H

#include "OverlayBox.h"
#include <hid\HIDListener.h>
#include "visual\Visual.h"

#include <string>
#include <core\MCBSystem.h>

namespace P3D {

	class Queue;
	class Animal;

	class HintBox : public OverlayBox, public HIDListener, public Visual, public FANLib::MCBSystem {

	protected:

		bool closed;
		bool closing;

		bool typewriter;
		Ogre::UTFString actualText;
		unsigned currentLength;
		double timeToNextChar;

		Ogre::Vector2 mediumSize;
		Ogre::Vector2 finalSize;
		int finalFontHeight;

		Queue* myQueue;

		void closeFinished();

	public:

		HintBox(const std::string& name, bool isTypewriter = false);
		virtual ~HintBox();

		enum CallBacks {
			CLOSING,
			_last
		};

		static const double APPEAR_TIME;
		static const double HOLD_TIME;
		static const double UNFOLD_TIME;
		static const double UNFOLD_TEXT_TIME;
		static const double SECS_PER_CHAR;		/// if typewriter

		virtual void setText(const std::string& name);

		bool isClosed() { return closed; };

		double delay;
		Animal* animal;

		void setFinalSize(double, double);			/// call prior to open
		void setFinalFontHeight(double);			/// call prior to open
		void setRelativePosition(double, double);	/// call <last>
		virtual void open();
		virtual void close();

		void open(void*);	/// open by 'Queue' callback

		virtual void playlistFinished(TPlaylist*);
		virtual void onMouseDown();
		virtual bool run(double time);
	};
}

#endif