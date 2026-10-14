// ---------
// TCubeFX - special tweening
// ---------

#pragma once
#ifndef TCubeFX_H
#define TCubeFX_H

#include "BasicTween.h"

namespace P3D {

	template <class Object, class Var>
	class TCubeFX : public BasicTween<Object, Var> {

	protected:

		double speed;
		Var stepValue;	/// cached calculation (optimization)

	public:

		// constructor
		TCubeFX(Object* obj, Var end, double endT, double _speed, const Var& (Object::*getMember)() const, void (Object::*setMember)(const Var&), double _delay = 0) :
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay), speed(_speed) {
		};

		template <class Getter, class Setter>
		TCubeFX(Object* obj, Var end, double endT, double _speed, Getter getMember, Setter setMember, double _delay = 0) :
		  BasicTween<Object, Var>(obj, end, endT, getMember, setMember, _delay), speed(_speed) {
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->BasicTween<Object, Var>::start();

			stepValue = (this->endValue - this->startValue);
		};

		virtual double _run(double addTime) {

			this->time += addTime;
			if (this->time >= this->endTime) {

				/// object <final value> set in tween's <destructor>!
				return this->time - this->endTime;

			} else {

				double y = cos( 4 * Pi * cos( this->time * speed ) );
				if (y >= 0.5) {
					y = 1;
				} else if (y <= -0.5) {
					y = 0;
				} else {
					y = 0.5;
				}

				this->apply(this->startValue + stepValue * y);
				return -1.;

			}
		};

	};

}
#endif