#include "StdAfx.h"
#include "game\ai\BallAI.h"

#include "game\ai\AIPVALinear.h"
#include "game\ai\AIPVAGravity.h"
#include "game\ai\AIPVAnimator.h"
#include "collision\CollisionObject.h"
#include "collision\CollisionExecuter.h"
#include "game\CosmosCreator.h"
#include "game\action\FallAction.h"
#include "game\action\StickAction.h"
#include "game\action\ReflectAction.h"
#include "game\action\NoAction.h"
#include "game\form\Form.h"
#include "game\form\BallForm.h"
#include "game\event\EventManager.h"
#include "game\event\LinkEvent.h"
#include "game\event\UnlinkEvent.h"
#include "game\event\InPlaceEvent.h"
#include "game\event\BallKilledEvent.h"
#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "OgreAddOns.h"
#include "BurstBoss.h"
#include "visual\VisualBoss.h"
#include "visual\BallBoom.h"

#include "sound\Sound.h"

#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <OgreParticleSystem.h>

namespace P3D {

	const double BallAI::TIME_TO_STICK = 0.1; /// = 3 frames at 60Hz

	BallAI::BallAI(GameplayObject* obj, BallAI::InternalState intState, Ogre::SceneNode* parent, const Ogre::Vector3& position, double inMass)
		: AIPosVel(obj, parent, position, false), myAction(0), internalState(OUT_OF_PLAY) {

		/// register for <BallCreator> usage
		GameplayBase* base = owner->getGameplayBase();
		base->insertBall(this->owner);

		/// parameterize
		mass = inMass;
		changeInternalState(intState);

		/// special initial state case
		switch (intState) {
			case STUCK:
				netStuckPosition = base->getNet()->reserveNode(OgreAddOns::getDerivedPosition(pos));
				break;
		}

		/// Sounds <!>
		Sound::BALL_BIG_POP->init();
		Sound::BALL_BOUNCE->init();
		Sound::BALL_BZZZT->init();
	}

	BallAI::~BallAI() {
		owner->getGameplayBase()->removeBall(this->owner); /// unregister from base
		changeAction(0);
		prepareToDie(); /// <!!!>
	}

	void BallAI::changeAction(Action* act) {
		delete myAction; /// it's OK if it's <Null>
		myAction = act;
	}

	AI::CheckForCollision BallAI::checkForCollision() {
		return checkCollision;
	}

	Action* BallAI::getAction() {
		return myAction;
	}

	void BallAI::changeInternalState(BallAI::InternalState inState) {

		/// was : check <previous> state for special actions
		//Net* net = owner->getGameplayBase()->getNet();
		//switch (internalState) {
		//	case MOVING_IN_PLACE:
		//	case STUCK:
		//		net->setNodeAvailable(netStuckPosition); /// reset node <availability>
		//		break; }

		/// set <new> state
		switch (inState) {

			case FLYING:
				changeAction(new ReflectAction(Action::LOW));
				checkCollision = AI::CAUSE;
				unlink();
				break;

			case MOVING_IN_PLACE:
				changeAction(new ReflectAction(Action::LOW));
				checkCollision = AI::ACCEPT;
				break;

			case STUCK:
				changeAction(new StickAction(Action::LOW)); /// attach my 'CollisionObject' (if any) to 'StickAction'
				checkCollision = AI::ACCEPT;
				if (form) link();
				break;

			case FALLING:
				changeAction(new NoAction());	/// does nothing to other objects
				checkCollision = AI::HYBRID;	/// this means "check against all other but Hybrid objects"
				unlink();
				break;

			case OUT_OF_PLAY:
			default: /// just in case
				changeAction(new NoAction());	/// } "invisible" to other objects
				checkCollision = AI::INVISIBLE;	/// }
				unlink();
				break;
		}

		internalState = inState;
	}

	void BallAI::receiveAction(Action* action) { /// incoming action is checked against <internal state> for appropriate reaction

		switch (action->getType()) {

			/// Action::NO_ACTION is ignored

			case Action::FALL:
				switch (internalState) {
					case BallAI::FALLING:				/// do nothing, already falling
					///case BallAI::MOVING_IN_PLACE:	/// <hopefully> MOVING_IN_PLACE is alright to accept FALLING
						return;
					default:
						onFall(action);
						return;
				}
				return;

			case Action::DESTROY:
				onDestroy(action); /// take it in right away
				return;

			case Action::REFLECT:
				switch (internalState) {
					case BallAI::FLYING:
						onReflect(action);
						return;
					case BallAI::FALLING:
						onDestroy(action);
						return;
					default:
						return;
				}
				return;

			case Action::STICK:
				switch (internalState) {
					case BallAI::FLYING:
						onStick(action);
						return;
					case BallAI::FALLING:
						onDestroy(action);
						return;
					default:
						return;
				}
				return;
		}
	}

	void BallAI::onFall(Action* action) {
		owner->getGameplayBase()->getNet()->releaseNode(netStuckPosition); /// release any previously tied position
		changeInternalState(BallAI::FALLING);
		double delay = ((FallAction*)action)->delay / this->pos->_getDerivedScale().length() * 2;
		owner->getGameplayBase()->getAIPVAnimator()->newAnime(new AIPVAGravity(this, -35), delay);
	}

	void BallAI::onReflect(Action* action) {

		const CollisionParams& collision = action->getCollision();

		/// speed always gets reflected regardless 'Action's <level>
		CollisionExecuter::reflectVelocity(vel, collision, mass);

		/// <visual> : do some sparks!
		double ballRadius;
		BallState* ballState = (BallState*)(this->owner->getFormState());
		GameplayBase* base = owner->getGameplayBase();
		if (ballState->getState() == FormState::BALL) {
			ballRadius = ballState->getBallRadius();
		} else {
			ballRadius = BallState::BALL_RADIUS;
		}
		/// - get sparks orientation
		Ogre::Vector3 sparksNomal = ( base->getRootNode()->getOrientation() * Ogre::Vector3(-1,1,-1) ) * collision.normal; /// THIS TOOK SOME TIME TO DISCOVER!!!
		Ogre::Quaternion sparksOrient = OgreAddOns::getOrientation( Ogre::Vector3(0,0,1), sparksNomal ); /// (0,0,1) defined in 'particle_system Burst/Sparks'
		base->getBurstBoss()->newBurst(BurstBoss::SPARKS, pos->getParentSceneNode(), pos->getPosition() + ballRadius * sparksNomal, sparksOrient);

		/// - sound
		Sound::BALL_BOUNCE->play( getWorldPosition() );
	}

	void BallAI::onStick(Action* action) {

		/// calculate stuck position and set as <unavailable> !!!
		Net* net = owner->getGameplayBase()->getNet();
		net->releaseNode(netStuckPosition); /// release any previously tied position
		std::pair<Ogre::Vector3, NetNode> pair = net->findAndReserveNode(OgreAddOns::getDerivedPosition(pos), action->getCollision().gameObj2->getCollisionObject());
		netStuckPosition = pair.second;

		/// <TODO> : animate-in-place & listen to "animation-end"...
		setVelocity(Ogre::Vector3::ZERO);
		changeInternalState(BallAI::MOVING_IN_PLACE);
		owner->getGameplayBase()->getAIPVAnimator()->newAnime(
			new AIPVALinear(this, OgreAddOns::worldToLocal(pos, &pair.first), TIME_TO_STICK),
			0, this
			);

		/// <visual> : do some sparks!
		const CollisionParams& collision = action->getCollision();
		double ballRadius;
		BallState* ballState = (BallState*)(this->owner->getFormState());
		GameplayBase* base = owner->getGameplayBase();
		if (ballState->getState() == FormState::BALL) {
			ballRadius = ballState->getBallRadius();
		} else {
			ballRadius = BallState::BALL_RADIUS;
		}
		/// - get sparks orientation
		Ogre::Vector3 sparksNomal = base->getRootNode()->getOrientation() * (-collision.normal); /// THIS TOOK SOME TIME TO DISCOVER!!!
		//was: = ( base->getRootNode()->getOrientation() * Ogre::Vector3(-1,-1,-1) ) * collision.normal;
		Ogre::Vector3 sparksTemp = sparksNomal * Ogre::Vector3(1,1,-1);
		Ogre::Quaternion sparksOrient = OgreAddOns::getOrientation( Ogre::Vector3(0,0,1), sparksTemp ); /// THIS ALSO TOOK LONG!!!
		base->getBurstBoss()->newBurst(BurstBoss::STUCK, pos->getParentSceneNode(), pos->getPosition() + ballRadius * sparksNomal, sparksOrient);

		/// - sound
		Sound::BALL_BZZZT->play( getWorldPosition() );
	}

	void BallAI::animeOver(AIPVAnime* anime) {
		setVelocity(Ogre::Vector3::ZERO);

		switch (internalState) {
			case MOVING_IN_PLACE:
				changeInternalState(BallAI::STUCK); /// <NOTE> : also tells this object to <link>, and sets speed to <0>
				owner->getGameplayBase()->getEventManager()->incomingEvent(new InPlaceEvent(this->owner));
				break;
		}
	}

	void BallAI::onDestroy(Action* action) {

		/// create an according <explosion> animation in my place as visual-feedback for my destruction

		GameplayBase* base = owner->getGameplayBase();

		/// - <boom>
		/// - do we have a <BallState> in order to get a boom <colour> (balltype)?
		BallState::BallType ballType;
		BallState* ballState = (BallState*)owner->getFormState();
		if (ballState->getState() == FormState::BALL) {
			ballType = ballState->getBallType();
		} else {
			ballType = BallState::WHITE; /// ... just in case
		}
		VisualBoss::getSingletonPtr()->insert( new BallBoom(base, ballType, getPosition(), 2) );


		/// - <burst>
		BurstBoss::Type burstType;
		switch (internalState) {
			case BallAI::FALLING:
				burstType = BurstBoss::BIG;
				Sound::BALL_BIG_POP->play( getWorldPosition() );
				break;
			default:
				burstType = BurstBoss::SMALL;
				break;
		}
		base->getBurstBoss()->newBurst(burstType, pos->getParentSceneNode(), pos->getPosition());

		/// this is a "hack" for 'MatchThree' to re-enable cannon in case I was destroyed by someone else while flying!
		if (internalState == FLYING) owner->getGameplayBase()->getEventManager()->incomingEvent(new BallKilledEvent(owner));

		prepareToDie();
	}

	void BallAI::prepareToDie() {
		if (!owner->dead) {
			/// - become "invisible" till I am destroyed
			owner->getGameplayBase()->getNet()->releaseNode(netStuckPosition); /// release any previously tied position
			changeInternalState(OUT_OF_PLAY); /// - this will <UNLINK> me, too (and cause <all> known side-effects)

			/// mark 'GameplayObject' owner as "dead"
			owner->dead = true;
		}
	}

	void BallAI::setWorldPosition(const Ogre::Vector3& position) {
#ifndef _DEPLOY
		if (internalState == STUCK) throw "BallAI BUG! Position set to STUCK Ball!"; /// DEBUG!!! Fortunately hasn't ever occured!
#endif
		OgreAddOns::setDerivedPosition(pos, &position);
	}

	bool BallAI::attachForm(Form* inForm) {

		if (AIPosVel::attachForm(inForm)) return true; /// remove previously attached 'Form' & attach to 'pos' & attach 'vel'

		// - set 'FormState'
		//form->setState(myState->copy()); // MUST PASS A COPY!!!!!

		/// "reset" <internal state> in case it is interdependent with 'Form' (like <STUCK>)
		changeInternalState(internalState);

		return false;
	}

}