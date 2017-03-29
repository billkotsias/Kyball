///////////////
// PreviewLevel - "preview" a level e.g show something that's changing before starting actual game
///////////////

#pragma once
#ifndef PreviewLevel_H
#define PreviewLevel_H

#include "visual\Visual.h"
#include <hid\HIDListener.h>

#include <string>

namespace P3D {

	class Queue;

	class PreviewLevel : public Visual, public HIDListener {

	protected:

		Queue* myQueue;

		Ogre::Camera* myCam;
		Ogre::Camera* origCam;
		double time;
		const char* const cam;

	public:

		PreviewLevel(const char* const _cam, double _time);
		virtual ~PreviewLevel();

		void start(void*);	/// start by 'Queue' callback
		void stop();		/// stop
		virtual bool run(double);

		virtual void onMouseDown();
	};
}

#endif