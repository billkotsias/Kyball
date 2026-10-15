// -------
// Tween - linear-time dependent tweening interface
// -------

#pragma once
#ifndef AbstractTween_H
#define AbstractTween_H

#include "ITween.h"

#include <functional>

namespace P3D {

	//template <class Object, class Var, const Var& (*get)(), void (*set)(const Var&)>
	template <class Object, class Var>
	class AbstractTween : public ITween {

	protected:

		Object* object;	/// pointer to object to be tweened
		double time;	/// current tween time
		double endTime;	/// end-of-tween time

		std::function<void(const Var&)> apply;	/// function setting the object's value

	public:

		// Exact signature selects among overloaded setters such as setScale / setSelfIllumination.
		AbstractTween(Object* obj, double endT, void (Object::*setMember)(const Var&), double _delay = 0) : ITween(_delay), object(obj), endTime(endT) {
			apply = [obj, setMember](const Var& value) { (obj->*setMember)(value); };
		};

		template <class Setter>
		AbstractTween(Object* obj, double endT, Setter setMember, double _delay = 0) : ITween(_delay), object(obj), endTime(endT) {
			apply = [obj, setMember](const Var& value) { (obj->*setMember)(value); };
		};

	};

}
#endif