#include "StdAfx.h"
#include "game\GameplayObject.h"

#include "game\action\Action.h"

#include "math\Math.h"
#include <utility>

namespace P3D {

	GameplayObject::GameplayObject(GameplayBase* base) : ai(0), form(0), gameplayBase(base), dead(false) {
	}

	void GameplayObject::setAI(P3D::AI* inAi) {
		if (ai == inAi) return;

		delete ai;
		ai = inAi;
		if (ai) ai->attachForm(form);
	}

	void GameplayObject::setForm(P3D::Form* inForm) {
		if (form == inForm) return;

		delete form;
		form = inForm;
		if (ai) ai->attachForm(form); /// form may be NULL
	}

	GameplayObject::~GameplayObject() {
		clearIncomingActions();
		setAI(0);	/// NOTE : first destroy <AI> because it needs to detach any attached <Form>;
		setForm(0);	/// now that <Form> is detached from any AI, it can be destroyed
	}

	void GameplayObject::clearIncomingActions() {
		for (int i = incomingActions.size() - 1; i >= 0; --i) {
			delete incomingActions.at(i); /// delete copy of incoming Action
		}
		incomingActions.clear();
	}

	void GameplayObject::receiveAction(Action* action) {
		incomingActions.push_back(action);
	}

	void GameplayObject::react() {

		/// send actions to AI (FIFO)
		for (unsigned int i = 0; i < incomingActions.size(); ++i) {
			ai->receiveAction(incomingActions.at(i)); /// if AI needs an action later, it should <make a copy itself>
		}

		clearIncomingActions(); /// clear now-old incoming actions
	}

	CollisionParams GameplayObject::checkCollision(P3D::GameplayObject *obj) {

		/// return immediately in the following cases :
		/// - checking against ourselves
		/// - a Form or CollisionObject is missing
		/// - a CollisionObject is not ready
		if (this == obj ||
			!(this->form)	|| !(this->form->getCollisionObject())	|| !(this->form->getCollisionObject()->isReady())	||
			!(obj->form)	|| !(obj->form->getCollisionObject())	|| !(obj->form->getCollisionObject()->isReady())
			)
			return CollisionParams::NO_COLLISION;

		return this->form->getCollisionObject()->checkCollision( obj->form->getCollisionObject() );
	}

	double GameplayObject::distance(P3D::GameplayObject *obj) {

		/// return immediately in the following cases :
		/// - checking against ourselves
		/// - a Form or CollisionObject is missing
		/// - a CollisionObject is not ready
		if (this == obj ||
			!(this->form)	|| !(this->form->getCollisionObject())	|| !(this->form->getCollisionObject()->isReady())	||
			!(obj->form)	|| !(obj->form->getCollisionObject())	|| !(obj->form->getCollisionObject()->isReady())
			)
			return Infinite;

		return this->form->getCollisionObject()->distance( obj->form->getCollisionObject() );
	}

	FormState* GameplayObject::getFormState() {
		if (form) return form->getState();
		return 0;
	}

}