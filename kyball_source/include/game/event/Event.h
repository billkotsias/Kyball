////////
// Event
////////

#pragma once
#ifndef Event_H
#define Event_H

namespace P3D {

	class Event {

	public:

		/* possible 'Event' types */;
		enum Type {

			LINK_ME,			/// an object manifests itself as "linkable" (for 'Rules' using "linking")
			UNLINK_ME,			/// an object is no longer "linkable"
			IN_PLACE,			/// an object has moved "in-place"
			BALL_KILLED,		/// a BALL was killed while flying
			MOVER_EVENT,		/// an object manifests itself as "movable" (for 'Rules' moving objects towards cannon)

			SHRINK_LEVEL,		/// current level is "shrinking"; used by <OpenCube>
			SHRINK_IN_2,		/// will shrink in 2 shots
			SHRINK_IN_1,		/// will shrink in 1 shot

			LOST,				/// player lost, anyone cares?
			WON,				/// player won
			COSMOS_FINISHED,	/// self-explanatory

			CANNON_SHOT,		/// cannon has just shot; used by <Rules>

			OBJECT_MODIFIED,	/// future : this should go in <GameplayObject> when <AI> or <Form> are changed

			_none
		};

	protected:

		Event::Type type; /// event type

		/* constructor */;
		Event(Event::Type inType) : type(inType) {};

	public:

		virtual ~Event() {};

		inline Event::Type getType() const { return type; };

		virtual Event* copy() { return new Event(type); }; /// remember to <delete> this copy when you no more need it

	};
}

#endif