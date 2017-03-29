#include "StdAfx.h"
#include "game\editor\BallEditor.h"

#include "hid\HIDBoss.h"
#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\GameplayCollection.h"
#include "game\net\Net.h"
#include "game\ai\BallAI.h"
#include "game\ai\BobAI.h"
#include "game\ai\BroAI.h"
#include "game\form\BallForm.h"
#include "game\form\BobForm.h"
#include "game\cannon\BallCreator.h"
#include "game\cannon\Cannon.h"
#include "OgreBuilder.h"
#include "OgreAddOns.h"

#include <core\Utils.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>

namespace P3D {

	const std::string BallEditor::ballTypeStr[BallState::MAX_TYPES + BallState::MAX_BOBS] = {
		"r", "g", "b", "c", "m", "y", "w", "k", "bob", "bro"
	};

	BallEditor::BallEditor(GameplayBase* inBase) : base(inBase), x(0), y(0), z(0) {

		net = base->getNet();
		collection = base->getCollection();
		
		Ogre::SceneNode* parent = base->getRootNode();
		node = parent->createChildSceneNode();
		entity = OgreBuilder::createEntity("BallEditor", "BallEditor.mesh", false, "", node);

		updatePosition();

		acceptKeyEvents(true);
	}

	BallEditor::~BallEditor() {
		OgreBuilder::destroyMovable(entity);
		OgreBuilder::destroySceneNode(node);
	}

	void BallEditor::updatePosition() {
		Ogre::Vector3 pos = net->netNodeToWorld(x,y,z);
		OgreAddOns::setDerivedPosition(node, &pos);
	}

	void BallEditor::createBall(BallState::BallType newType) {
		GameplayObjectsIterator gameIterator;
		GameplayObject* gameplayObject;

		/// find gameplayobjects that have the same position as the 'BallEditor's cursor
		int ox,oy,oz;
		BallState::BallType ballType = BallState::_last; /// marked as "no Ball found at cursor position"

		if (gameIterator.begin(base, gameplayObject)) {
			while (gameIterator.next()) {
				/// check if it's a 'Ball' we are talking about
				BallState* ballState = (BallState*)(gameplayObject->getFormState());
				if (ballState->getState() == FormState::BALL) {
					net->getNearestNode(gameplayObject->getAI()->getWorldPosition(), ox,oy,oz);
					if (x == ox && y == oy && z == oz) {
						ballType = ballState->getBallType();
						break;
					}
				}
			}
		}

		/// was a ball found at cursor's position?
		if (ballType != BallState::_last) {
			/// - delete found ball
			///gameplayObject->dead = true; /// with this, change won't be <saved>, because it will be destroyed <after> the save()
			delete gameplayObject;
			gameIterator.removeGameplayObject();
		}

		/// was there a request to delete or to create a new ball?
		if (newType < BallState::UNCHANGED) {
			GameplayObject* obj = collection->storeGameplayObject(new GameplayObject(base));
			Ogre::Vector3 objectPos = OgreAddOns::worldToLocalParent(base->getRootNode(), &(net->netNodeToWorld(x,y,z)) );
			BallAI* ballAI;
			BallForm* ballForm;
			if (newType < BallState::BOB) {
				ballAI = new BallAI(obj, BallAI::STUCK, base->getRootNode(), objectPos);
				ballForm = new BallForm(base, base->getBBSet(GameplayBase::BALLS), newType);
			} else if (newType == BallState::BOB) {
				ballAI = new BobAI(obj, base->getRootNode(), objectPos);
				ballForm = new BobForm(base, base->getBBSet(GameplayBase::BOBS), BallState::BOB);
			} else if (newType == BallState::BRO) {
				ballAI = new BroAI(obj, base->getRootNode(), objectPos);
				ballForm = new BobForm(base, base->getBBSet(GameplayBase::BOBS), BallState::BRO);
			}
			obj->setAI(ballAI);
			obj->setForm(ballForm);
			ballAI->setVelocity(Ogre::Vector3::ZERO);
		}

		/// <save> changes
		save();
	}

	void BallEditor::onKeyEvent() {

		/// update position
		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isKeyPressed(OIS::KC_LEFT)) x += 1;
		if (hid->isKeyPressed(OIS::KC_RIGHT)) x -= 1;
		if (hid->isKeyPressed(OIS::KC_UP)) z += 1;
		if (hid->isKeyPressed(OIS::KC_DOWN)) z -= 1;
		if (hid->isKeyPressed(OIS::KC_HOME)) y += 1;
		if (hid->isKeyPressed(OIS::KC_END)) y -= 1;
		updatePosition();

		/// create new ball or replace previous one
		if (hid->isKeyPressed(OIS::KC_R)) createBall(BallState::RED);
		if (hid->isKeyPressed(OIS::KC_G)) createBall(BallState::GREEN);
		if (hid->isKeyPressed(OIS::KC_B)) createBall(BallState::BLUE);
		if (hid->isKeyPressed(OIS::KC_C)) createBall(BallState::CYAN);
		if (hid->isKeyPressed(OIS::KC_M)) createBall(BallState::MAGENTA);
		if (hid->isKeyPressed(OIS::KC_Y)) createBall(BallState::YELLOW);
		if (hid->isKeyPressed(OIS::KC_W)) createBall(BallState::WHITE);
		if (hid->isKeyPressed(OIS::KC_K)) createBall(BallState::BLACK);

		if (hid->isKeyPressed(OIS::KC_V)) createBall(BallState::BOB);
		if (hid->isKeyPressed(OIS::KC_X)) createBall(BallState::BRO);
		if (hid->isKeyPressed(OIS::KC_D)) createBall(BallState::UNCHANGED); /// means "delete"

		/// <save> created level
		if (hid->isKeyPressed(OIS::KC_S)) save();
	}

	void BallEditor::save() {

		std::string indent = "\t\t";

		/// create new text to copy to clipboard
		/// - start
		GameplayObjectsIterator gameIterator;
		GameplayObject* gameplayObject;
		Cannon* cannon = base->getCannon();
		int ox,oy,oz;

		text = indent + "balls [\n";
		indent += "\t";

		/// - write balls
		if (gameIterator.begin(base, gameplayObject)) {
			while (gameIterator.next()) {
				/// check if it's a 'Ball' we are talking about
				BallState* ballState = (BallState*)(gameplayObject->getFormState());
				if (ballState->getState() == FormState::BALL && gameplayObject->getAI() != cannon->getNextBallAI()) {
					net->getNearestNode(gameplayObject->getAI()->getWorldPosition(), ox,oy,oz);
					text += indent + "[ " + FANLib::Utils::toString(ox) + " " + FANLib::Utils::toString(oy) + " " + FANLib::Utils::toString(oz) +
						" c::" + ballTypeStr[ballState->getBallType()] + " ]\n";

				}
			}
		}

		/// - end
		indent.resize(indent.size() - 1);
		text += indent + "]\n";

		/// <save> produced text

		/// - copy to clipboard
		HGLOBAL h;
		LPTSTR arr;
		int textSize = text.size() + 1;

		h=GlobalAlloc(GMEM_MOVEABLE, textSize);
		arr=(LPTSTR)GlobalLock(h);
		strcpy_s((char*)arr, textSize, text.c_str());
		GlobalUnlock(h);

		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, h);
		CloseClipboard();
	}

}