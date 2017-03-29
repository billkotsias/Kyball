/////////
// Action - basis of all gameplay events
/////////

#pragma once
#ifndef Action_H
#define Action_H

#include "collision\CollisionParams.h"

namespace P3D {

	class Action {

	public:

		/* possible 'Action' types */;
		enum Type {
			NO_ACTION,	/// (another type could be translated into this)
			STICK,		/// stick with me
			FALL,		/// do a "free-fall"
			DESTROY,	/// die
			REFLECT,	/// reflect your speed
		};

		/* intended for use with 'ActionInt' */;
		enum Level {
			MIN		= 0,

			LOW		= 10,
			MEDIUM	= 100,
			HIGH	= 1000,

			MAX		= 0x7FFFFFFF,
		};

		Action::Type getType() const;

		/* remember to delete this copy when you no more needed !!! */;
		virtual Action* copy();

		virtual ~Action(); /// <WATCH IT> !!!

		Action* embedCollision(CollisionParams); /// returns this Action* (for convenience to the user)
		inline CollisionParams getCollision() { return collisionParams; };

	protected:

		/* type of this action */;
		Action::Type type;

		/* additionally embedded info */;
		CollisionParams collisionParams;

		// constructor
		Action(Action::Type typ);

	};
}

#endif