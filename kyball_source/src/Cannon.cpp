#include "StdAfx.h"
#include "game\cannon\Cannon.h"

#include "hid\HIDBoss.h"
#include "game\Camera.h"
#include "game\GameplayObject.h"
#include "game\GameplayBase.h"
#include "game\cannon\BallCreator.h"
#include "game\cannon\Aim.h"
#include "game\cannon\CannonSwarm.h"
#include "game\ai\BallAI.h"
#include "game\form\BallForm.h"
#include "game\event\CannonShotEvent.h"
#include "game\event\EventManager.h"
#include "tween\TLinear.h"
#include "sound\Sound.h"
#include "visual\CallEvery.h"

#include "P3DApp.h"
#include "P3DWindow.h"
#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "OgreAddOns.h"
#include "BurstBoss.h"
#include "sound\SoundBoss.h"

#include "math\Math.h"
#include <math.h>
#include <string>

#include <OgreParticleSystem.h>

namespace P3D {

	const double Cannon::MAX_HORIZONTAL_ROTATION	= 70; /// degrees
	const double Cannon::MAX_VERTICAL_ROTATION		= 70; /// degrees
	const double Cannon::START_SCALE = 0.55;
	const double Cannon::END_SCALE = 0.75;
	const double Cannon::CANNON_HEIGHT = 0.2;
	const double Cannon::FADEIN_SWARM = 0.4;
	const double Cannon::GOTO_SWARM = 0.3;
	const double Cannon::FADEOUT_SWARM = 0.4;
	const double Cannon::FADEOUT_TUBE = 0.15;
	const double Cannon::TOTAL_SWARM = Cannon::GOTO_SWARM + Cannon::FADEOUT_SWARM + 0.02;
	const double Cannon::AIM_SPEED = 0.9999999999 * .5;

	const std::vector<Ogre::ColourValue> Cannon::ballColours = BallState::initBallColours(1, 0.3, 0.3, 0.3);


	Cannon::Cannon(Ogre::SceneNode* pos, Ogre::SceneNode* tub) : cannonNode(pos), tubeNode(tub),
		horizontalRotation(0), verticalRotation(0), timeToNextShot(0), nextBall(0), ballCreator(0), aim(0),
		fireworksSound(0), particleSystem(0)
	{
	}

	void Cannon::playFireworks() {
		particleSystem = base->getBurstBoss()->newBurst( BurstBoss::BASE_FIREWORKS, cannonNode );
		particleSystem->setCallBack(this, &Cannon::playFireworksSound);
	}

	void Cannon::playFireworksSound(void*) {
		if (!fireworksSound) fireworksSound = Sound::getScripted("gameplay\\firework2");
		fireworksSound->play( OgreAddOns::getDerivedPosition(cannonNode) );
	}

	void Cannon::init(GameplayBase* base) {
		this->base = base;

		camera = base->getCamera();
		eMan = base->getEventManager();

		/// below 3 entities could as well belong to a Collection (only kept to destroy later!)
		cannon = OgreBuilder::createEntity("cannon_base" + base->getID(), "cannon_base.mesh", false, "", cannonNode);
		tube = OgreBuilder::createEntity("cannon_tube" + base->getID(), "cannon_tube.mesh", false, "", tubeNode);
		tubePass = tube->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);

		cannonViewCamera = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createCamera("cannonViewCamera");
		cannonViewCamera->synchroniseBaseSettingsWith( base->getCamera()->getOgreCamera() );
		cannonViewCamera->rotate( Ogre::Vector3::UNIT_Y, Ogre::Radian( Ogre::Degree(180) ) );
		cannonViewCamera->setFOVy( cannonViewCamera->getFOVy() / 0.6 );
		tubeNode->attachObject(cannonViewCamera);

		resetOrientation();

		/// cache positions
		Ogre::Vector3 temp = cannonNode->getPosition();
		cannonNode->translate(0,END_SCALE / 2. + CANNON_HEIGHT,0);
		startPos = OgreAddOns::getDerivedPosition(cannonNode);	/// <start> position of fade-in cannonSwarm
		cannonNode->setPosition(temp);

		endPos = OgreAddOns::getDerivedPosition(tubeNode);		/// <end> position of fade-in cannonSwarm

		acceptMouseEvents(true); /// if it hasn't been already, <HIDBoss> will be created here

		/// visual elements
		swarmName = "cannon_swarm" + base->getID();
		swarm[0] = 0;
		swarm[1] = 0;

		bbset = base->getBBSet(GameplayBase::BALLS);

		Sound::BALL_FIRE->init();
	}

	void Cannon::stopFireworks(bool lastToo) {
		if (lastToo && fireworksSound) {
			fireworksSound->stopLastPlaying();
		}
		if (particleSystem) {
			base->getBurstBoss()->disableSystem( particleSystem );
			particleSystem = 0;
		}
	}

	Cannon::~Cannon() {

		//acceptMouseEvents(false);

		deleteTweens(); /// must be called prior to any tweens objects' deletion

		delete swarm[0];
		delete swarm[1];

		OgreBuilder::destroyMovable(cannon);
		OgreBuilder::destroyMovable(tube);
		OgreBuilder::destroyMovable(cannonViewCamera);

		attachBallCreator(0);
		attachAim(0);

		stopFireworks(true);
	}

	void Cannon::reset() {

		deleteTweens(); /// must be called prior to any tweens objects' deletion

		/// - swarms
		delete swarm[0];
		delete swarm[1];
		swarm[0] = new CannonSwarm(swarmName + "_0", cannonNode);
		swarm[1] = new CannonSwarm(swarmName + "_1", cannonNode);

		currentSwarm = 0;

		/// - 'nextBall'
		ballCreator->calcFutureBallType();
		tubePass->setSelfIllumination(ballColours[ballCreator->getFutureBallType()]);
		createNextBall();
		nextBallBillboard = 0; /// <!!!>
		ballCreator->calcFutureBallType(); /// !!! <!!!> !!!

		fadeinSwarm();
		timeToNextShot = timeBetweenShots;

		/// - 'Enabled'
		disablers.clear();
		f_Enabled = true;

		/// - orientation
		resetOrientation();
		setMaximumDeviation(1.0);

		stopFireworks(false);
		setCannonView(false);
	}

	void Cannon::resetOrientation() {
		horizontalRotation = 0;
		verticalRotation = 0;
		orientCannon();
	}

	void Cannon::setMaximumDeviation(double deviation) {
		maxHorizontalRotation = MAX_HORIZONTAL_ROTATION * deviation;
		maxVerticalRotation = MAX_VERTICAL_ROTATION * deviation;
	}

	void Cannon::attachBallCreator(P3D::BallCreator *creator) {
		delete ballCreator;
		ballCreator = creator;
		nextBall = 0;
	}

	void Cannon::attachAim(P3D::Aim *inAim) {
		delete aim;
		aim = inAim;
	}

	void Cannon::setTimeBetweenShots(double time) {
		timeBetweenShots = (time > TOTAL_SWARM) ? time : TOTAL_SWARM;
	}

	void Cannon::onMouseMove() {
		static int x, y;

		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isButtonDown(HIDBoss::MOUSE_RIGHT)) return; /// don't rotate cannon if user is trying to rotate the camera <!>
		hid->getMouseMove(x, y);

		/// <TODO> : have one function for both axes!
		double xMove = pow(abs((double)x) / 6., 1.0); /// <TODO> : calibrate mouse <3. = MAX SPEED! 10. = BETTER !!!>
		if (x < 0) xMove = - xMove;
		horizontalRotation	-= xMove;
		horizontalRotation	= FANLib::Math::bound(horizontalRotation, maxHorizontalRotation);
		/// was : horizontalRotation	= FANLib::Math::bound(horizontalRotation, MAX_HORIZONTAL_ROTATION);

		double yMove = pow(abs((double)y) / 6., 1.0); /// <TODO> : calibrate mouse
		if (y < 0) yMove = - yMove;
		verticalRotation	+= yMove;
		verticalRotation	= FANLib::Math::bound(verticalRotation , maxVerticalRotation);
		// was : verticalRotation	= FANLib::Math::bound(verticalRotation , MAX_VERTICAL_ROTATION);

		orientCannon();
	}

	void Cannon::orientCannon() {
		static Ogre::Quaternion quat;

		rotMatrix.FromEulerAnglesYXZ(Ogre::Degree(horizontalRotation), Ogre::Degree(verticalRotation), Ogre::Radian(0));
		quat.FromRotationMatrix(rotMatrix);
		tubeNode->setOrientation(quat);

		/// add camera orientation
		camera->setAdditionalOrientation(horizontalRotation / MAX_HORIZONTAL_ROTATION, verticalRotation / MAX_VERTICAL_ROTATION);
	}

	void Cannon::onMouseDown()
	{
		if (!f_Enabled) return; /// no input accepted
		HIDBoss* hid = HIDBoss::getSingletonPtr();

		if (timeToNextShot <= 0 && (nextBallBillboard == 0) && hid->isButtonDown(HIDBoss::MOUSE_LEFT))
		{
			eMan->incomingEvent(new CannonShotEvent()); /// send event of new shot (cannon gets disabled by MatchThree!!!)

			/// <TODO> : set ball speed <externally> through a function
			/// give initial velocity to 'nextBall'
			BallAI* ballAI = (BallAI*)nextBall->getAI();
			ballAI->setVelocity(Ogre::Vector3(0, 0, 17.99 / P3DApp::CYCLES_PER_SEC) * rotMatrix.Inverse()); /// <17.99> was : <18.>
			ballAI->changeInternalState(BallAI::FLYING);
			((BallForm*)nextBall->getForm())->changeBBSet(bbset);
			
			/// bid farewell to last <nextBall> and welcome the <new one>
			swarm[currentSwarm]->gotoLaunch(endPos, GOTO_SWARM, FADEOUT_SWARM);
			currentSwarm = 1 - currentSwarm;

			createNextBall();
			aim->enabled = false;
			timeToNextShot += timeBetweenShots;

			fadeAll();

			Sound::BALL_FIRE->play( endPos );
		}
	}

	void Cannon::disable(FANLib::DisableID id)
	{
		this->FANLib::Enabled::disable(id);
		//setCannonView(false);
	}

	void Cannon::setCannonView(bool set)
	{
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		if (set) {
			if (sceneBase->getCamera() != camera->getOgreCamera()) return; /// probably "FlyIn" state
			SceneBaseCreator::getSingletonPtr()->setCamera(cannonViewCamera);
			SoundBoss::getSingletonPtr()->setCamera(cannonViewCamera);
			//aim->enabled = false; /// put here for one-frame-showing fix
			aim->radiusScaler = 0.5;

		} else {
			if (sceneBase->getCamera() != cannonViewCamera) return; /// probably "FlyIn" state
			SceneBaseCreator::getSingletonPtr()->setCamera( camera->getOgreCamera() );
			SoundBoss::getSingletonPtr()->setCamera( camera->getOgreCamera() );
			aim->radiusScaler = 1.0;
		}
	}

	void Cannon::onMouseChange()
	{
		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isButtonPressed(HIDBoss::MOUSE_MIDDLE))
		{
			setCannonView(true);

		} else if (hid->isButtonReleased(HIDBoss::MOUSE_MIDDLE))
		{
			setCannonView(false);
		}
	}

	void Cannon::createNextBall() {

		/// - get ball from creator
		nextBall = ballCreator->create();
		BallAI* ballAI = (BallAI*)nextBall->getAI();
		BallForm* ballForm = (BallForm*)nextBall->getForm();
		nextBallBillboard = ballForm->getBillboard();

		/// - set position and velocity
		ballAI->setWorldPosition(endPos);
		ballAI->setVelocity(Ogre::Vector3(0, 0, 0));

		/// - adjust 'Aim' to match new ball
		aim->setPointParams(((BallState*)ballForm->getState())->getBallRadius(), nextBall->getMass());
	}

	void Cannon::run(double time)
	{
		aim->enabled = false;

		/// Keep order below so that within a frame, 'timeToNextShot' may be negative, and if it goes '+= timeBetweenShots', it will be smaller!
		if (timeToNextShot < 0) timeToNextShot = 0;
		if (timeToNextShot > 0) {
			timeToNextShot -= time;
		} else {
			//if (f_Enabled && SceneBaseCreator::getSingletonPtr()->getCamera() != cannonViewCamera) aim->enabled = true;
			if (f_Enabled) aim->enabled = true;
		}
	}

	void Cannon::runAim(int cycles) {
		aim->run(cycles, Ogre::Vector3(0, 0, AIM_SPEED) * rotMatrix.Inverse());
	}

	void Cannon::calcFutureBall() {
		ballCreator->calcFutureBallType();
		fadeinSwarm();
	}

	//

	void Cannon::fadeinSwarm() {
		swarm[currentSwarm]->fadeIn(startPos,ballColours[ballCreator->getFutureBallType()], START_SCALE, END_SCALE, FADEIN_SWARM);
	}

	void Cannon::fadeAll() {

		TPlaylist* list;
		const Ogre::ColourValue& endCol = ballColours[ballCreator->getFutureBallType()];

		/// list 1 : tube fades
		list = new TPlaylist();
		list->add(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			tubePass, Ogre::ColourValue::ZERO, FADEOUT_TUBE, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
		list->add(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			tubePass, endCol, FADEOUT_SWARM, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination, GOTO_SWARM - FADEOUT_TUBE)
			);
		registerList(list);

		/// list 2 : tube scales
		list = new TPlaylist();
		list->add(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			tubeNode, Ogre::Vector3(0,0,3), FADEOUT_TUBE, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		list->add(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			tubeNode, Ogre::Vector3::UNIT_SCALE, 0.1, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		registerList(list);

		/// tween 3 : nextBall scale
		nextBallBillboard->setDimensions(0.,0.);
		TPlaylist* nextBallFade = registerTween(new TLinear<Cannon, Ogre::Vector2>(
			this, Ogre::Vector2::UNIT_SCALE, Cannon::FADEOUT_SWARM, &Cannon::getBBDim, &Cannon::setBBDim, Cannon::GOTO_SWARM)
			);
		nextBallFade->id = nextBallFadeID; /// set an <ID> to this 'TPlaylist' to distinguish from others!
	}

	const Ogre::Vector2& Cannon::getBBDim() const {
		static Ogre::Vector2 bbDims;
		bbDims.x = nextBallBillboard->getOwnWidth();
		bbDims.y = nextBallBillboard->getOwnHeight();
		return bbDims;
	}

	void Cannon::setBBDim(const Ogre::Vector2& dims) {
		nextBallBillboard->setDimensions(dims.x, dims.y);
	}

	void Cannon::playlistFinished(TPlaylist* list) {
		if (list->id == nextBallFadeID) { /// is this the marked TPlaylist?
			nextBallBillboard->resetDimensions();
			nextBallBillboard = 0;
		}
		this->TPlayed::playlistFinished(list); /// <!!!>
	}

	BallAI* Cannon::getNextBallAI() { return (BallAI*)nextBall->getAI(); };
}
