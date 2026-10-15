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
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay) {
		};

		template <class Getter, class Setter>
		TLinear(Object* obj, Var end, double endT, Getter getMember, Setter setMember, double _delay = 0) :
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay) {
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->BasicTween<Object, Var>::start();
			stepValue = (this->endValue - this->startValue) / this->endTime; /// optimization : cache step
		};
		/// - linear value interpolation
		virtual double _run(double addTime) {

			this->time += addTime;
			if (this->time >= this->endTime) {

				///was: (object->*set)(endValue); /// object <final value> set in tween's <destructor>!
				return this->time - this->endTime;

			} else {

				this->apply(this->startValue + (stepValue * this->time) );
				return -1;

			}
		};

	};

}
#endif