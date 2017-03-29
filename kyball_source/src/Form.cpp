#include "StdAfx.h"
#include "game\form\Form.h"
#include "game\form\GenericState.h"

#include "SceneBaseCreator.h"
#include "collision\CollisionObject.h"
#include "OgreBuilder.h"

#include <OgreSceneNode.h>

namespace P3D {

	Form::Form() : collisionObject(0), formState(0), pivot(0) {
		pivot = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createSceneNode();
		setState(new GenericState());
	}

	Form::~Form() {

		/// destroy <CollisionObject>
		changeCollisionObject(0);

		/// destroy <FormState>
		setState(0);

		/// clean-up and destroy 'pivot' the <gereric> way
		OgreBuilder::destroySceneNode(pivot);
	}

	void Form::setState(FormState* newState) {
		delete formState;
		formState = newState;
	}

	void Form::prepareCollisionObject() {
		pivot->_update(true, true);
		if (collisionObject) collisionObject->cacheParams();
	}

	CollisionObject* Form::getCollisionObject() {
		return collisionObject;
	}

	void Form::changeCollisionObject(P3D::CollisionObject *newCol) {
		delete collisionObject;
		collisionObject = newCol;
	}
}