/////
// AI - defines total behaviour of a gameplay object; may also affect the object's appearance
/////

#pragma once
#ifndef AI_H
#define AI_H

#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class Action;
	class Form;
	class FormState;
	class GameplayObject;
	class CollisionObject;

	class AI {

	protected:

		GameplayObject* owner;	/// "owner" of this AI
		Form* form;				/// register a depiction for this 'AI'
		double mass;			/// a mass "weight" for every object!

		/* internal service members */;
		bool linked;
		CollisionObject* getCollisionObject(); /// get 'CollisionObject' of attached 'Form' (if any)

	public:

		enum CheckForCollision {
			CAUSE,		/// AI may cause a collision with other objects
			ACCEPT,		/// AI accepts collisions from other objects but doesn't cause itself
			INVISIBLE,	/// AI doesn't cause or accept collisions
			HYBRID,		/// check for collision against CAUSE and ACCEPT, but <NOT> against other 'HYBRID's
		};

		AI(GameplayObject*);
		virtual ~AI();

		virtual CheckForCollision checkForCollision()	{return AI::INVISIBLE;};
		virtual bool checkForFutureCollision()			{return false;};		/// does object require to be informed of future collisions with other objects?


		// run AI for a portion of a cycle
		// => percent = percent of a cycle; should be [0, 1]
		//	  NOTE : take care to make AI's thought time-dependent : stupid for short times, smart for larger times!
		virtual void run(double) {};

		// function called once at the end of a cycle
		virtual void endOfCycle() {};


		// manifest behaviour to other objects
		virtual Action* getAction();

		// another object is "acting" upon this object : interpret action and react accordingly
		// => pair = a pair of incoming action from other object + any probable CollisionParams
		virtual void receiveAction(Action*) {};


		// another object WILL "act" upon this object in a future game cycle
		/// NOTE : this event may <NOT> be received if checkForFutureCollision() returns <false>; if you need this event, return <true> in your derived class
		// => action = incoming action from object
		virtual void futureAction(Action* action) {};


		// attach a "depiction" to this "brain"
		// <= return 'true' to quit the derived classes recursive function calling!!!
		virtual bool attachForm(Form*);


		// get AI's 'world' position (if any)
		virtual Ogre::Vector3 getWorldPosition()	{ return Ogre::Vector3::ZERO; };


		/// getters/setters
		inline double getMass()						{ return mass; };
		inline void setMass(double inMass)			{ mass = inMass; }; /// change "inline -> virtual" if a subclass doesn't want anyone to mess with its mass
		inline void setOwner(GameplayObject* obj)	{ owner = obj; };

	};
}

#endif