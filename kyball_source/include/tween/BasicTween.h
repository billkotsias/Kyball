// -------
// Tween - base of linear-time dependent tweens
// -------

#pragma once
#ifndef BasicTween_H
#define BasicTween_H

#include "AbstractTween.h"

namespace P3D {

	//template <class Object, class Var, const Var& (*get)(), void (*set)(const Var&)>
	template <class Object, class Var>
	class BasicTween : public AbstractTween<Object, Var> {

	protected:

		Var startValue;	/// starting value to "tween"
		Var endValue;	/// end value of "tween"

		const Var& (Object::*get)() const;	/// function getting the object's value

	public:

		// constructor
		BasicTween(Object* obj, Var endV, double endT, const Var& (Object::*getMember)() const, void (Object::*setMember)(const Var&), double _delay = 0) :
		  AbstractTween(obj, endT, setMember, _delay), endValue(endV), get(getMember) {
		};

		virtual ~BasicTween() {
			(object->*set)(endValue); /// set <final value> to tweened object!
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			time = 0;
			startValue = (object->*get)();
		};

	};

}
#endif