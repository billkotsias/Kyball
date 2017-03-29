////////////
// CallEvery
////////////

#pragma once
#ifndef CallEvery_H
#define CallEvery_H

#include "visual\Visual.h"
#include "visual\VisualBoss.h"

namespace P3D {

	class Sound;

	class ICallEvery : public Visual {

	public:

		ICallEvery(double initDelay, unsigned int times, double _repeatEvery) :
			timeToCall(initDelay),
			timesToCall(times),
			repeatEvery(_repeatEvery)
		{
			if (repeatEvery < 0) repeatEvery = initDelay;
			VisualBoss::getSingletonPtr()->insert(this);
		};
		virtual ~ICallEvery() {};

	protected:

		unsigned int timesToCall;
		double timeToCall;
		double repeatEvery;

		virtual bool run(double time);
		virtual void callFunction() = 0;
	};

	///

	template<class Class=void>
	class CallEvery : public ICallEvery {

	public:

		typedef void(Class::*MFunction)();

		CallEvery(Class* _object, MFunction _function, double initDelay, unsigned int timesToCall = 1, double repeatEvery = -1.) :
			ICallEvery(initDelay, timesToCall, repeatEvery), function(_function), object(_object)
		{
		};
		virtual ~CallEvery() {};

		virtual void callFunction() { (object->*function)(); }

	protected:

		MFunction function;
		Class* object;
	};

	///

	template<>
	class CallEvery<void> : public ICallEvery {

	public:

		typedef void(*Function)();

		CallEvery(Function _function, double initDelay, unsigned int timesToCall = 1, double repeatEvery = -1.) :
			ICallEvery(initDelay, timesToCall, repeatEvery), function(_function)
		{
		};

		virtual ~CallEvery() {};

		virtual void callFunction() { function(); }

	protected:

		Function function;
	};
}

#endif