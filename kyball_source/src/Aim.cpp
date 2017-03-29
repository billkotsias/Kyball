#include "StdAfx.h"
#include "game\cannon\Aim.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\ai\AI.h"
#include "game\form\Form.h"
#include "game\action\Action.h"
#include "game\net\Net.h"
#include "collision\Sphere.h"
#include "collision\CollisionExecuter.h"
#include "OgreBuilder.h"
#include "OgreAddOns.h"

#include <OgreBillboardSet.h>
#include <OgreColourValue.h>
#include <OgreSceneNode.h>
#include <utility>

namespace P3D {

	const double Aim::POINTS_BEHIND = 1.4;
	const double Aim::SHADOW_SIZE = 0.20;	/// 0.25
	const double Aim::NORMAL_SIZE = 0.5;	/// 1.
	const double Aim::REFLECT_SIZE = 0.75;	/// 2.
	const Ogre::ColourValue Aim::REFLECT_COL = Ogre::ColourValue(.8,1.,1.);

	Aim::Aim(GameplayBase* inBase, int pNum) : ShadowCaster(inBase->getShadowManager(), ShadowCaster::BLOB), countStep(0.0099999999), count(0),
		enabled(false), base(inBase), radiusScaler(1.0)
	{

		// geometry
		sphereNode = base->getRootNode()->createChildSceneNode(); /// must follow this for correct <shadowing> and <BallAI> compatibility !
		sphereVel = base->getRootNode()->createChildSceneNode();
		Sphere* sphere = new Sphere(sphereNode, 0);
		sphere->setVelocity(sphereVel);
		startPosition = OgreAddOns::worldToLocal(sphereNode, &base->getBallEndPos());
		/// collisionObject->checkFutureCollision = true; <?!?> could be made faster..??? MAYBE, MAYBE NOT

		// logic
		sphereObj = new GameplayObject(base);
		sphereObj->setAI(new AI(sphereObj));
		Form* form = new Form();
		form->changeCollisionObject(sphere);
		sphereObj->setForm(form);

		// depiction
		bbset = base->getBBSet(GameplayBase::AIM);
		setPointsNum(pNum);
		defaultSize.x = bbset->getDefaultWidth();
		defaultSize.y = bbset->getDefaultHeight();

		// extra help depiction
		helpbbset = base->getBBSet(GameplayBase::AIM_HELP);
		helpbbset->clear();
		helpbbset->createBillboard(0,0,0);
		helpbbset->setVisible(false);
	}

	Aim::~Aim() {
		OgreBuilder::destroySceneNode(sphereNode);
		delete sphereObj;
	}

	void Aim::setPointsNum(int num) {
		int current = bbset->getNumBillboards();
		/// remove excess
		for (int i = current - num; i > 0; --i) {
			bbset->removeBillboard(current - 1);
		}
		/// add some more
		for (int i = num - current; i > 0; --i) {
			bbset->createBillboard(0,0,0);
		}
	}

	int Aim::getPointsNum() {
		return bbset->getNumBillboards();
	}

	void Aim::setPointParams(double radius, double inMass) {
		((Sphere*)sphereObj->getCollisionObject())->setLocalRadius(radius);
		sphereObj->getAI()->setMass(inMass);
	}

	void Aim::drawBillboard(unsigned int index, const Ogre::Vector3& pos, const double& radius, const Ogre::ColourValue& colour) {
		Ogre::Billboard* bb = bbset->getBillboard(index);
		bb->setPosition(pos);
		bb->setColour(colour);
		Ogre::Vector2 bbSize = defaultSize * radius * radiusScaler;
		bb->setDimensions(bbSize.x, bbSize.y);
		pointsDrawn.push_back(bb);
	}

	void Aim::run(int cycles, Ogre::Vector3 velocity) {

		base->groupCollisionObjects();
		base->prepareCollisionObjects();

		/// make all billboards "invisible"
		for (int i = bbset->getNumBillboards() - 1; i >= 0; --i) {
			bbset->getBillboard(i)->setColour(Ogre::ColourValue(1,1,1,0));
		}
		pointsDrawn.clear(); /// remove their shadows

		helpbbset->setVisible(false);

		if (!enabled) return;


		/// advance points' animation
		count += cycles * countStep;
		while (count > 1) { --count; }

		/// get 1st point's position
		sphereNode->setPosition(startPosition + (-POINTS_BEHIND + count) * velocity); /// <+1>
		sphereVel->setPosition(velocity);

		std::vector<GameplayObject*>* vectors[3];
		vectors[0] = &base->getAcceptCollision();
		vectors[1] = &base->getCauseCollision(); /// that's a fine-looking effect!
		vectors[2] = 0; /// end

		/// travel point forward, checking for collision every step
		/// <NOTE> : __maybe__ it's faster to check for <future> collision and advance to that future point, than checking <every> point sequentially
		Ogre::ColourValue nextCol(Ogre::ColourValue::White);

		int pointsNum = bbset->getNumBillboards();
		int point = pointsNum + 1;
		double time = 1;
		while (--point)
		{
			double nextBBSize = NORMAL_SIZE * 0.5 * (pointsNum - point + count - 0.5); /// size of 1st drawn point : count = [0,0.5]=>0, (0.5,1]=>(0,NORMAL_SIZE]
			if (nextBBSize < 0) nextBBSize = 0; else if (nextBBSize > NORMAL_SIZE) nextBBSize = NORMAL_SIZE;

			static double tolerance = 0.00001;

			CollisionParams nearest;
			Sphere* sphere = (Sphere*)sphereObj->getForm()->getCollisionObject();
			sphere->cacheParams();

			/// check against all objects
			int arrayNum = -1;
			while (vectors[++arrayNum]) {

				std::vector<GameplayObject*>* array = vectors[arrayNum];
				for (unsigned int i = 0; i < array->size(); ++i) {

					if (i == 0) {
						i = i;
					}

					GameplayObject* gameObj = array->at(i);
					CollisionObject* gameColObj = gameObj->getCollisionObject();
					if (gameColObj) {
						CollisionParams params = sphere->checkCollision(gameColObj); /// "optimization"
						if (params < nearest) {
							nearest = params;
							nearest.gameObj1 = sphereObj;
							nearest.gameObj2 = gameObj;
						}
					}
				}
			}

			if (nearest.isValid() && nearest.time < time) { // && nearest.time >= tolerance) {
				Action* action = nearest.gameObj2->getAction();
				std::pair<Ogre::Vector3, NetNode> pair;

				switch (action->getType()) {
					case Action::REFLECT:
						sphereNode->translate(velocity * nearest.time);
						time -= nearest.time;
						nearest.calculateLambda();
						CollisionExecuter::reflectVelocity(sphereVel, nearest, sphereObj->getMass());
						velocity = sphereVel->getPosition();
						nextBBSize = REFLECT_SIZE;
						nextCol = REFLECT_COL;
						break;
					case Action::DESTROY:
						sphereNode->translate(velocity * nearest.time);
						point = 1;	/// end of line
						break;
					case Action::STICK:
						sphereNode->translate(velocity * nearest.time);
						point = 1;	/// end of line
						/// <TODO> : check if we have the same <colour> and do an effect!
						pair = base->getNet()->findNode(sphereNode->_getDerivedPosition(), nearest.colObj2);
						helpbbset->setVisible(true);
						helpbbset->getBillboard(0)->setPosition( OgreAddOns::worldToLocalParent( helpbbset->getParentSceneNode(), &pair.first) );
						nextBBSize = 0; /// don't draw this last point
						break;
					default:
						sphereNode->translate(velocity * nearest.time);
						time -= nearest.time;
						break;
				}
			} else {
				sphereNode->translate(velocity * time);
				time = 1;
			}

			drawBillboard(pointsNum - point, sphereNode->getPosition(), nextBBSize, nextCol);

			/// reset default params
			nextBBSize = NORMAL_SIZE;
			nextCol = Ogre::ColourValue::White;
		}
	}

	bool Aim::updateShadow(Ogre::Vector2* position, Ogre::Vector2* scaling) {
		if (pointsDrawn.empty()) return false;

		Ogre::Billboard* bb = pointsDrawn.back();
		const Ogre::Vector3& bbPos = bb->getPosition();
		position->x = bbPos.x;
		position->y = bbPos.z;
		*scaling = SHADOW_SIZE * bb->getColour().a;

		pointsDrawn.pop_back();
		if (!pointsDrawn.empty()) return true;
		return false;
	}
}