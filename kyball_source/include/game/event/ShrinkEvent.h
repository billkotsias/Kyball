////////
// ShrinkEvent
////////

#pragma once
#ifndef ShrinkEvent_H
#define ShrinkEvent_H

#include "Event.h"

namespace P3D {

	class ShrinkEvent : public Event {

	protected:

		double time;		/// shrink duration
		double shrinkage;	/// shrink amount

	public:

		/* constructor */;
		ShrinkEvent(double t, double s) : Event(Event::SHRINK_LEVEL), time(t), shrinkage(s) {};

		inline double getTime() { return time; };
		inline double getShrinkage() { return shrinkage; };

		/// overrides
		virtual Event* copy() { return new ShrinkEvent(time, shrinkage); };
	};
}

#endif