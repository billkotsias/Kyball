// ---------
// ITween - tween interface
// ---------

#pragma once
#ifndef ITween_H
#define ITween_H

namespace P3D {

	class ITween;

	class TweenListener {
	public:
		virtual void tweenFinished(ITween*) = 0;
	};

	class ITween {

	protected:

		TweenListener* listener;
		double delay;	/// delay to start

	public:

		// constructor
		ITween(double _delay) : listener(0), delay(_delay) {};

		virtual ~ITween() {};

		inline void setListener(TweenListener* listen) { listener = listen; };
		inline TweenListener* getListener() { return listener; }; /// used by <Players>

		// initialize tweening
		virtual void start() = 0;

		// run an amount of time - take delay into account
		// => time units to run
		// <= time units left over; if >= 0, this tween has finished
		double run(double time) {
			if (delay > 0) {
				delay -= time;
				if (delay >= 0) {
					return -1;
				} else {
					time = -delay;
				}
			}
			return _run(time);
		};

		// run an amount of time
		// => time units to run
		// <= time units left over; if >= 0, this tween has finished
		virtual double _run(double) = 0;

	};

}
#endif