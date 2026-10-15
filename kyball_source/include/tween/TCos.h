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
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay) {
		};

		template <class Getter, class Setter>
		TCos(Object* obj, Var end, double endT, Getter getMember, Setter setMember, double _delay = 0) :
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay) {
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->BasicTween<Object, Var>::start();

			invEndTime = Pi / 2. / this->endTime;
			stepValue = (this->endValue - this->startValue);
		};

		virtual double _run(double addTime) {

			this->time += addTime;
			if (this->time >= this->endTime) {

				/// object <final value> set in tween's <destructor>!
				return this->time - this->endTime;

			} else {

				this->apply(this->startValue + stepValue * (1. - cos(this->time * invEndTime)) );
				return -1.;

			}
		};

	};

}
#endif