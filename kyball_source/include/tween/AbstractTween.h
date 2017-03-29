// -------
// Tween - linear-time dependent tweening interface
// -------

#pragma once
#ifndef AbstractTween_H
#define AbstractTween_H

#include "ITween.h"

namespace P3D {

	//template <class Object, class Var, const Var& (*get)(), void (*set)(const Var&)>
	template <class Object, class Var>
	class AbstractTween : public ITween {

	protected:

		Object* object;	/// pointer to object to be tweened
		double time;	/// current tween time
		double endTime;	/// end-of-tween time

		void (Object::*set)(const Var&);	/// function setting the object's value

	public:

		// constructor
		AbstractTween(Object* obj, double endT, void (Object::*setMember)(const Var&), double _delay = 0) : ITween(_delay), object(obj), endTime(endT), set(setMember) {
		};

	};

}
#endif