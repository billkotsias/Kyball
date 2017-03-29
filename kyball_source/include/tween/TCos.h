// ---------
// TCos - cosine tweening
// ---------

#pragma once
#ifndef TCos_H
#define TCos_H

#include "BasicTween.h"

namespace P3D {

	template <class Object, class Var>
	class TCos : public BasicTween<Object, Var> {

	protected:

		double invEndTime;
		Var stepValue;	/// the value's constant step / time unit

	public:

		// constructor
		TCos(Object* obj, Var end, double endT, const Var& (Object::*getMember)() const, void (Object::*setMember)(const Var&), double _delay = 0) :
		  BasicTween(obj, end, endT, getMember, setMember, _delay) {
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->BasicTween<Object, Var>::start();

			invEndTime = Pi / 2. / endTime;
			stepValue = (endValue - startValue);
		};

		virtual double _run(double addTime) {

			time += addTime;
			if (time >= endTime) {

				/// object <final value> set in tween's <destructor>!
				return time - endTime;

			} else {

				(object->*set)(startValue + stepValue * (1. - cos(time * invEndTime)) );
				return -1.;

			}
		};

	};

}
#endif