// -------
// Tween - base of linear-time dependent tweens
// -------

#pragma once
#ifndef BasicTween_H
#define BasicTween_H

#include "AbstractTween.h"
#include <functional>

namespace P3D {

	//template <class Object, class Var, const Var& (*get)(), void (*set)(const Var&)>
	template <class Object, class Var>
	class BasicTween : public AbstractTween<Object, Var> {

	protected:

		Var startValue;	/// starting value to "tween"
		Var endValue;	/// end value of "tween"

		std::function<Var()> read;	/// function getting the object's value

	public:

		BasicTween(Object* obj, Var endV, double endT, const Var& (Object::*getMember)() const, void (Object::*setMember)(const Var&), double _delay = 0) :
		  AbstractTween<Object, Var>(obj, endT, setMember, _delay), endValue(endV) {
			read = [obj, getMember]() -> Var { return (obj->*getMember)(); };
		};

		template <class Getter, class Setter>
		BasicTween(Object* obj, Var endV, double endT, Getter getMember, Setter setMember, double _delay = 0) :
		  AbstractTween<Object, Var>(obj, endT, setMember, _delay), endValue(endV) {
			read = [obj, getMember]() -> Var { return (obj->*getMember)(); };
		};

		virtual ~BasicTween() {
			this->apply(endValue); /// set <final value> to tweened object!
		};

		// overrides
		/// - get object's original state
		virtual void start() {
			this->time = 0;
			startValue = read();
		};

	};

}
#endif