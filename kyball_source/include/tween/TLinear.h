// ---------
// TLinear - linear tweening
// ---------

#pragma once
#ifndef TLinear_H
#define TLinear_H

#include "BasicTween.h"

namespace P3D {

	template <class Object, class Var>
	class TLinear : public BasicTween<Object, Var> {

	protected:

		Var stepValue;	/// the value's constant step / time unit

	public:

		// constructor
		TLinear(Object* obj, Var end, double endT, const Var& (Object::*getMember)() const, void (Object::*setMember)(const Var&), double _delay = 0) :
		  BasicTween(obj, end, endT, getMember, setMember, _delay) {
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->BasicTween<Object, Var>::start();
			stepValue = (endValue - startValue) / endTime; /// optimization : cache step
		};
		/// - linear value interpolation
		virtual double _run(double addTime) {

			time += addTime;
			if (time >= endTime) {

				///was: (object->*set)(endValue); /// object <final value> set in tween's <destructor>!
				return time - endTime;

			} else {

				(object->*set)(startValue + (stepValue * time) );
				return -1;

			}
		};

	};

}
#endif