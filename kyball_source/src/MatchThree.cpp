#include "StdAfx.h"
#include "game\rule\MatchThree.h"

#include "game\event\BallKilledEvent.h"
#include "game\event\GOEvent.h"
#include "game\event\ShrinkEvent.h"
#include "game\event\ShrinkIn2Event.h"
#include "game\event\ShrinkIn1Event.h"
#include "game\event\EventManager.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\net\Net.h"

#include "game\ai\AIPosVel.h"
#include "game\ai\AIPVAnime.h"
#include "game\ai\AIPVALinear.h"
#include "game\ai\AIPVAnimator.h"
#include "game\form\Form.h"
#include "game\form\BallState.h"

#include "game\action\FallAction.h"
#include "game\action\DestroyAction.h"
#include "game\action\StickAction.h"

#include "game\animal\Animal.h"
#include "game\cannon\Cannon.h"

#include "tween\TLinear.h"
#include "sound\Sound.h"

#include <core\Utils.h>
#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include <deque>
#include <math.h>

#include <core\Utils.h>

namespace P3D {

	const double MatchThree::TOLERANCE = BallState::BALL_RADIUS / 10.;
	const double MatchThree::TIME_TO_ADVANCE = 0.7;

	const unsigned int MatchThree::POP_SCORE = 21;
	const unsigned int MatchThree::FALL_SCORE = 111;
	const unsigned int MatchThree::MAX_FALLING_SCORE = 999999; // was: 21

	const double MatchThree::ANXIOUS_EXTENT = 1./3.;	/// lower than this, there's anxiety!
	const double MatchThree::SMILE_EXTENT = 2./3.;		/// greater than this, there are smiles!

	MatchThree::MatchThree(GameplayBase* inBase, unsigned int match) : Rule(inBase), matches(match), checkResult(Rule::NOTHING), disableID(FANLib::DisableID::get()) {

		EventManager* eMan = base->getEventManager();
		watchEvent(eMan,Event::LINK_ME);
		watchEvent(eMan,Event::UNLINK_ME);
		watchEvent(eMan,Event::IN_PLACE);
		watchEvent(eMan,Event::CANNON_SHOT);
		watchEvent(eMan,Event::BALL_KILLED);
		watchEvent(eMan,Event::MOVER_EVENT);

		advanceEvery = 8; /// <TODO> : pass-in this value (which is read from script, with a default value of 7-8)
		toAdvance = advanceEvery;

		/// Sounds <!>
		Sound::BALL_SMALL_POP->init();
		Sound::CUBE_SHRINK_IN1->init();
		Sound::CUBE_SHRINK_IN2->init();
		Sound::CUBE_SHRINKING->init();
	}

	MatchThree::~MatchThree() {
		deleteTweens(); /// must be called prior to any tweens objects' deletion
	}

	Rule::Result MatchThree::perCycleCheck() {
		return checkResult;
	}

	void MatchThree::enableCannon() {
		base->getCannon()->enable(disableID);
		//std::cout << "MatchThree cannon ENABLED! " + FANLib::Utils::toString(base->getCannon()->f_IsEnabled()) + "\n";
	}

	void MatchThree::disableCannon() {
		base->getCannon()->disable(disableID);
		//std::cout << "MatchThree_cannon_DISABLED!" + FANLib::Utils::toString(base->getCannon()->f_IsEnabled()) + "\n";
	}

	void MatchThree::nextCannonRound() {

		base->getCannon()->calcFutureBall();

		/// shrink level?
		--toAdvance;

		if (toAdvance == 2) {
			base->getEventManager()->incomingEvent(new ShrinkIn2Event());
			Sound::CUBE_SHRINK_IN2->play();
		} else if (toAdvance == 1) {
			base->getEventManager()->incomingEvent(new ShrinkIn1Event());
			Sound::CUBE_SHRINK_IN1->play();
		}

		if (toAdvance == 0) {
			toAdvance = advanceEvery;
			advanceToCannon(); /// cannon will be re-enabled when shrinking finishes!
			Sound::CUBE_SHRINKING->play();
		} else {
			enableCannon();
		}
	}

	// NOTE : This rule advances all linked objects towards the cannon, every X "cannon shots". When X is reached, the rule :
	//	a) disables the cannon, b) waits for an "IN_PLACE" or "flying-ball-touched-BackPlane" event, then does the advancement. Then, cannon is re-enabled.
	void MatchThree::incomingEvent(P3D::Event *event) {

		GameplayObject *obj;

		switch (event->getType()) {

			case Event::CANNON_SHOT:
				disableCannon(); /// disable cannon till ball arrives!
				break;

			case Event::LINK_ME:
				obj = ((GOEvent*)event)->getGameplayObject();
				link(obj);
				insertMover(obj); /// automaticall set as 'mover'
				break;

			case Event::UNLINK_ME:
				obj = ((GOEvent*)event)->getGameplayObject();
				if (unlink(obj)) checkFalling(obj); /// check for "falling" balls only if ball wasn't already unlinked
				removeMover(obj); /// automaticall unset as 'mover' (?)
				break;

			case Event::BALL_KILLED:
				Sound::BALL_SMALL_POP->play( ((BallKilledEvent*)event)->getGameplayObject()->getAI()->getWorldPosition() );
				nextCannonRound();
				break;

			case Event::IN_PLACE:
				obj = ((GOEvent*)event)->getGameplayObject();
				matchThree(obj);
				nextCannonRound();
				break;

			case Event::MOVER_EVENT:
				obj = ((GOEvent*)event)->getGameplayObject();
				insertMover(obj);
				break;
		}

	}

	void MatchThree::advanceToCannon() {

		/// advancement quantity
		Net* net = base->getNet();
		double zDiff = net->getZDiff();

		/// - advance <movers>
		for (std::map<GameplayObject*, bool>::iterator it = moveToCannon.begin(); it != moveToCannon.end(); ++it) {
			advanceToCannon(it->first, zDiff);
		}

		/// - advance <all linked objects>
		//for (std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator it = links.begin(); it != links.end(); ++it) {	
		//	advanceToCannon(it->first, zDiff);
		//}

		/// - advance <Net>
		Ogre::Vector3 endPos = net->getOrigin()->getPosition();
		endPos.z -= zDiff;
		registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			net->getOrigin(), endPos, TIME_TO_ADVANCE, &Ogre::SceneNode::getPosition, &Ogre::SceneNode::setPosition)
			);

		/// - inform <shrinkable> 
		base->getEventManager()->incomingEvent(new ShrinkEvent(TIME_TO_ADVANCE, zDiff));
	}

	void MatchThree::advanceToCannon(GameplayObject* obj, double amount) {
		AIPosVel* ai = (AIPosVel*)obj->getAI();
		Ogre::Vector3 final = ai->getPosition();
		final.z -= amount;

		base->getAIPVAnimator()->newAnime( new AIPVALinear(ai, final, TIME_TO_ADVANCE), 0, this );
	}

	void MatchThree::animeOver(AIPVAnime* anime) {
		checkLose(anime->getObject());
	}

	void MatchThree::playlistFinished(TPlaylist* list) {
		this->TPlayed::playlistFinished(list); /// <!!!>
		enableCannon();
		checkAnimalEmotion(true);
	}

	void MatchThree::checkLose(AIPos *ai) {
		static double tolerance = -0.1; /// in case its -> -0 
		double aiZPos = ai->getPosition().z;

		if (aiZPos < tolerance) {
			checkResult = Rule::LOSE; /// on the next perCycleCheck(), we will respond with "LOSE"
		}
	}

	void MatchThree::checkWin() {
		bool winner = true; /// winner till proved otherwise!
		for (std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator it = links.begin(); it != links.end(); ++it) {
			if (it->first->getFormState()->getState() == FormState::BALL) {
				winner = false; /// there are still other balls linked
				break;
			}
		}

		if (winner) {
			checkResult = Rule::WIN; /// HOORAAAAH !
		}
	}

	void MatchThree::link(P3D::GameplayObject *obj) {

		unlink(obj);
		std::map<GameplayObject*, bool>& objLinks = links[obj];

		obj->getForm()->prepareCollisionObject(); /// <NOTE> : IS THIS <ENOUGH> OR SHOULD ALL OBJECTS BE UPDATED SOMEWHERE ALTOGETHER <?!!?>

		for (std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator it = links.begin(); it != links.end(); ++it) {
			if (obj->distance(it->first) <= TOLERANCE) {
				objLinks[it->first] = true;
				it->second[obj] = true;
			}
		}
	}

	bool MatchThree::unlink(GameplayObject* obj) {

		/// is object in 'links' ?
		std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator i = links.find(obj);
		if (i == links.end()) return false; /// nothing to unlink

		//std::cout << "Unlink : " << obj << "\n";
		std::map<GameplayObject*, bool>& objLinks = i->second;

		for (std::map<GameplayObject*, bool>::iterator it = objLinks.begin(); it != objLinks.end(); ++it) {
			std::map<GameplayObject*, bool>& itLinks = links[it->first];
			itLinks.erase(obj); /// remove object from its links
		}
		links.erase(obj); /// remove object's links

		return true;
	}

	void MatchThree::matchThree(P3D::GameplayObject *startingObj) {

		/// check if object has a 'ball' form
		BallState* ballState = ((BallState*)(startingObj->getFormState()));
		if (ballState->getState() != FormState::BALL) return; /// oops! Not a Ball!

		BallState::BallType ballType = ballState->getBallType(); /// get Ball's type/colour

		/// "reach" all indirectly-linked balls of the same colour with 'startingObj'
		std::deque<GameplayObject*> sameColour;
		sameColour.push_back(startingObj);

		/// deque is expanded every iteration as long as there are new balls to add
		for (unsigned int i = 0; i < sameColour.size(); ++i) {

			GameplayObject* object = sameColour.at(i);					/// get next object in deque
			std::map<GameplayObject*, bool>& objLinks = links[object];	/// get object's links
			
			/// add linked objects if they have same colour and if they haven't already been added
			for (std::map<GameplayObject*, bool>::iterator it = objLinks.begin(); it != objLinks.end(); ++it) {
				GameplayObject* linkedObj = it->first;

				/// is linked object a ball?
				BallState* linkedObjState = ((BallState*)(linkedObj->getFormState()));
				if (linkedObjState->getState() != FormState::BALL) continue; /// oops! Not a Ball!

				if (
					linkedObjState->getBallType() == ballType &&
					FANLib::Utils::rFind<std::deque<GameplayObject*>, GameplayObject*>(sameColour, sameColour.size(), linkedObj) < 0	/// not yet added!
					) {
						sameColour.push_back(linkedObj);
				}
			}
		}

		/// do we have a "match-three" case?
		unsigned int matched = sameColour.size();
		if (matched < matches) {
			checkAnimalEmotion(false);
			checkLose((AIPos*)(startingObj->getAI())); /// ...no. Check instead if we lose because of it!
			return;
		}

		// perform 'pop' sound!
		Sound::BALL_SMALL_POP->play( startingObj->getAI()->getWorldPosition() );

		// determine level of happiness for Animal
		Animal::Animation happiness = Animal::_none;
		if (matched >= matches + 1) {
			happiness = Animal::happy1; /// if booms > minimum
		}

		// add score!
		{
			unsigned int countedForScore = 0;
			for (int i = sameColour.size() - 1; i >= 0; --i) {
				if (((BallState*)sameColour.at(i)->getFormState())->getCountForScore()) ++countedForScore;
			}
			base->score += countedForScore * countedForScore * /*countedForScore **/ POP_SCORE;
		}
		
		/// destroy balls, but first <UNLINK> them because we want to do 1 ball-falling check, right here!
		for (int i = matched - 1; i >= 0; --i) {
			GameplayObject* object = sameColour.at(i);
			unlink(object);											/// 1st
			object->receiveAction(new DestroyAction(Action::HIGH));	/// 2nd
			object->react();										/// 3rd
		}

		// check for falling balls after boom
		unsigned int fallingNum = 0, countForScore = 0;
		checkFalling(startingObj, &fallingNum, &countForScore);

		// determine additional level of happiness for Animal!
		if (fallingNum >= 4) {
			happiness = Animal::happy3;
		} else if (fallingNum > 0) {
			happiness = Animal::happy2;
		}

		// add score!
		{
			unsigned int fallingCeil = (countForScore > MAX_FALLING_SCORE) ? MAX_FALLING_SCORE : countForScore;
			base->score += fallingCeil * fallingCeil * /*fallingCeil * */ FALL_SCORE;
		}

		// make animal happy
		if (happiness != Animal::_none) base->getAnimal()->event(happiness);
	}

	void MatchThree::checkFalling(GameplayObject* startingObj, unsigned int* fallingNum, unsigned int* countForScore) {

		std::deque<GameplayObject*>		toBeChecked; /// copy 'links' here; we can't directly iterate 'links' cause it changes every iteration through unlink()
		std::map<GameplayObject*, bool>	checked;
		std::deque<GameplayObject*>		linked; /// fill with all indirectly linked objects

		for (std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator it = links.begin(); it != links.end(); ++it) {
			toBeChecked.push_back(it->first);
		}

		for (int k = toBeChecked.size() - 1; k >= 0; --k) {

			/// get next ball not already checked
			GameplayObject* object = toBeChecked.at(k);
			if (checked.find(object) == checked.end()) {
				checked[object] = true;
				linked.clear();
				linked.push_back(object);
			} else {
				continue; /// object already checked
			}

			/// get all indirect object's links
			for (unsigned int j = 0; j < linked.size(); ++j) {
				
				/// add all object's direct links that are not already checked
				std::map<GameplayObject*, bool>& objLinks = links[ linked.at(j) ];

				for (std::map<GameplayObject*, bool>::iterator i = objLinks.begin(); i != objLinks.end(); ++i) {
					GameplayObject* lObj = i->first;
					if (checked.find(lObj) == checked.end()) {
						checked[lObj] = true;
						linked.push_back(lObj);
					}
				}
			}

			/// check if objects are fixed or not
			/// <NOTE> : "hacked" solution : objects are 'fixed' if "high-level sticky"
			/// Better solution : send a "MOVE" action to objects and let them decide if they should move..?
			int j;
			for (j = linked.size() - 1; j >= 0; --j) {
				Action* action = linked.at(j)->getAI()->getAction(); /// no need to check if AI exists, it's LINKED!
				if ( (action->getType() == Action::STICK && ((StickAction*)action)->getStickiness() == Action::HIGH) ) break;
			}
			//std::cout << "ALL LINKS = " << linked.size() << "\n";
			//std::cout << "FALLING = " << (j<0) << "\n";

			if (j < 0) { /// falling!
				if (fallingNum) *fallingNum += linked.size();

				/// make balls fall, but first <UNLINK> them because we don't want to do any more ball-falling check!
				for (j = linked.size() - 1; j >= 0; --j) {
					GameplayObject* fallingObject = linked.at(j);
					if (countForScore && ((BallState*)linked.at(j)->getFormState())->getCountForScore()) ++(*countForScore);

					/// - a visual effect
					double delay;
					if (startingObj) {
						delay = startingObj->distance(fallingObject)/ 25;
					} else {
						delay = 0;
					}

					unlink(fallingObject);									/// 1st
					fallingObject->receiveAction(new FallAction(delay));	/// 2nd
					fallingObject->react();									/// 3rd
				}
			}
		}

		checkAnimalEmotion(false);
		checkWin(); /// because 'checkFalling()' is always checked during these actions : "matchThree"/"unlink ball", we will check if there are no more balls here
	}

	void MatchThree::checkAnimalEmotion(bool sadness) {

		/// cube's extent
		double cubeExtent = base->levelZ + base->getNet()->getOrigin()->getPosition().z; /// net z position <= 0

		/// find closest ball
		double closest = Infinite;
		for (std::map<GameplayObject*, bool>::iterator it = moveToCannon.begin(); it != moveToCannon.end(); ++it) {
			double zPos = ((AIPos*)it->first->getAI())->getPosition().z;
			if (zPos < closest) closest = zPos;
		}
		// NOTE : old check only cattered for balls, now we catter for all "movers" towards cannon!!!
		//for (std::map<GameplayObject*, std::map<GameplayObject*, bool> >::iterator it = links.begin(); it != links.end(); ++it) {
		//	if (it->first->getFormState()->getState() == FormState::BALL) {
		//		double zPos = ((AIPos*)it->first->getAI())->getPosition().z;
		//		if (zPos < closest) closest = zPos;
		//	}
		//}

		double ballsExtent = closest / cubeExtent;

		Animal::Animation emotion = Animal::even;
		if (ballsExtent <= ANXIOUS_EXTENT) {
			if (sadness) {
				emotion = Animal::sad;
			} else {
				emotion = Animal::anxious;
			}
		} else if (ballsExtent > SMILE_EXTENT) {
			emotion = Animal::smiling;
		}
		base->getAnimal()->event(emotion);
	}

}