///////
// Form - the on-screen appearance and internal geometry of an object
///////

#pragma once
#ifndef Form_H
#define Form_H

namespace P3D {

	class FormState;
	class CollisionObject;

	class Form {

	protected:

		FormState* formState;				/// current 'FormState' set

		Ogre::SceneNode* pivot;				/// the root node for this form
		CollisionObject* collisionObject;	/// object's collision-checking geometry

		/// NOTE : The <CollisionObject> can be a container of itself, so it's OK to reference it here; it's not limiting.
		///		   On the other hand, <Ogre::MovableObject> is not a container, so it's left to the 'Form's child class to decide what kind of
		///		   container to use (e.g std::deque), if at all.

	public:

		Form();
		virtual ~Form();

		void prepareCollisionObject();					/// prepare for collision
		void changeCollisionObject(CollisionObject*);

		CollisionObject* getCollisionObject();
		Ogre::SceneNode* getPivot() { return pivot; };

		virtual void update() {};						/// update on-screen appearance
		virtual void setState(FormState*);				/// external message (e.g from an <AI>) that alters 'Form'
		FormState* getState() { return formState; };	/// get currently set 'FormState'; needed by <Rules> !!!

	};
}

#endif