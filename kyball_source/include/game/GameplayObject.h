/////////////////
// GameplayObject - the basis of all gameplay (composite object)
/////////////////

#pragma once
#ifndef GameplayObject_H
#define GameplayObject_H

#include "game\ai\AI.h"
#include "game\action\NoAction.h"
#include "game\form\Form.h"
#include "collision\CollisionObject.h"
#include "collision\CollisionParams.h"

#include <deque> /// support for many incoming actions!!!
#include <OgreVector3.h>

namespace P3D {

	class AI;
	class Form;
	class Action;
	class GameplayBase;
	class FormState;
	class CollisionObject;

	class GameplayObject {

	protected:

		AI* ai;		/// object behaviour	} These two are implicitly interconnected through 'Ogre::SceneNode's
		Form* form;	/// object appearance	}

		GameplayBase* gameplayBase; /// keep record of "parent"; currently used by <BallAI> (through its 'owner' member)

		std::deque<Action*> incomingActions; /// keep copies of incoming actions till reaction-time
		void clearIncomingActions();

		friend class Gameplay;		/// has direct access to our 'AI' and 'Form'
		friend class GameplayBase;	/// same here

	public:

		GameplayObject(GameplayBase*);
		~GameplayObject();

		bool dead;	/// object is to be removed (as soon as possible)

		inline GameplayBase* getGameplayBase() { return gameplayBase; };

		/* object playtime modifiers */;
		void setAI(AI*);
		void setForm(Form*);

		inline AI* getAI()		{ return ai; };
		inline Form* getForm()	{ return form; };

		/// "shortcuts" to 'AI' and 'Form' members
		inline double getMass() {
			if (ai) return ai->getMass();
			return 0;
		};
		inline Action* getAction() {
			static NoAction noAction;
			if (ai) return ai->getAction();
			return &noAction;
		};
		inline Ogre::Vector3 getWorldVelocity() {
			if (form) {
				CollisionObject* colObj = form->getCollisionObject();
				if (colObj) return colObj->getWorldVelocity();
			}
			return Ogre::Vector3::ZERO;
		};
		inline CollisionObject* getCollisionObject() {
			if (form) return form->getCollisionObject();
			return 0;
		};
		inline void update() { if (form) form->update(); };
		inline void prepareCollisionObject() { if (form) form->prepareCollisionObject(); };

		// another object is "acting" upon this object : I must DELETE this action when I no longer need it!!!
		// => pointer to action
		void receiveAction(Action*);

		// react to incoming event(s)
		void react();

		/* handle collision between 'GameplayObjects' "from within" */;
		CollisionParams checkCollision(GameplayObject*);
		double distance(GameplayObject*);

		/* get 'GameplayObject' 'Form's 'FormState' !!!  */;
		/* indicates what kind of 'Form' this object has */;
		/* + reveals other info e.g Sphere Radius!       */;
		FormState* getFormState();

	};
}

#endif