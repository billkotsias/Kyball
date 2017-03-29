#include "StdAfx.h"
#include "visual\FlyIn.h"

#include "game\GameplayBase.h"
#include "game\animal\Animal.h"

#include "Queue.h"
#include "P3DWindow.h"
#include "OgreAddOns.h"
#include "SceneBaseCreator.h"
#include "sound\SoundBoss.h"
#include "visual\VisualBoss.h"
#include "gui\HintBox.h"
#include "gui\SubScore.h"

#include "P3DScripts.h"
#include <fsl\FSLClass.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>

#include <hid\HIDBoss.h>
#include "map\SubMap.h" /// should be generic, as it's oversimplistic

namespace P3D {

	const char* const FlyIn::FLY_IN = "FlyIn";
	const unsigned int FlyIn::MAX_SHOWN = 2;
	const double FlyIn::FAST_FORWARD_MUL = 5.;

	FlyIn::FlyIn(GameplayBase* base) : timesShown(0), anim(0), hintBox(0), myQueue(0), skipText(0), control(NOTHING) {
		gameplayBase = base;
	}

	void FlyIn::start(void* queue) {

		myQueue = (Queue*)queue;
		if (timesShown++ >= MAX_SHOWN) {
			myQueue->next();
			return;
		}

		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();
		try {
			anim = sceneMan->createAnimationState(FLY_IN); /// an Ogre::Animation named as 'FLY_IN' must exist
			myCam = sceneMan->getCamera(FLY_IN);
		} catch(...) {
			anim = 0;
			myQueue->next(); /// fly-in data doesn't exist
			return;
		}
		anim->setLoop(false);
		anim->setEnabled(true);

		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		origCam = sceneBase->getCamera();
		myCam->setAspectRatio( origCam->getAspectRatio() );
		myCam->setNearClipDistance( origCam->getNearClipDistance() );
		myCam->setFarClipDistance( origCam->getFarClipDistance() );
		sceneBase->setCamera(myCam);
		SoundBoss::getSingletonPtr()->setCamera(myCam);
		myCam->getParentSceneNode()->setVisible(true);

		if (!hintBox) {
			hintBox = new HintBox("Overlay/AnimalBox", true);
			hintBox->delay = 0;
			hintBox->setFinalSize(0.9,0.25);
			hintBox->setFinalFontHeight(0.05);
			hintBox->setRelativePosition(0.95, 0.8);
			hintBox->setCallBack(HintBox::CLOSING, this, &FlyIn::end);
		}

		if (!skipText) skipText = new SubMap("flyin");
		skipText->open();
		skipText->wEdit->setCaption( P3DScripts::p3dVars->getRoot()->getCString("flyinSkip") );

		VisualBoss::getSingletonPtr()->insert(this);
		acceptMouseEvents(true);
		acceptKeyEvents(true);

		gameplayBase->getSubScore()->close();
	}

	FlyIn::~FlyIn() {
		end(); /// a bit riskai to have this here
		delete hintBox;
		delete skipText;
	}

	bool FlyIn::run(double time) {

		switch (control) {
			case FAST_FORWARD:
				time *= FAST_FORWARD_MUL;
				break;
			case PAUSE:
				time = 0.;
				break;
			case REWIND:
				time *= -FAST_FORWARD_MUL;
				break;
			case RESET:
				time = - anim->getTimePosition();
				break;
		}
		control = NOTHING; /// reset control; input must be continuous

		anim->addTime(time);
		if (anim->getTimePosition() < 0) anim->setTimePosition(0);

		if (anim->hasEnded())
		{
			FANLib::FSLClass* cosmosfsl = P3DScripts::p3dCosmoi->getRoot()->getClass( gameplayBase->getCosmos().c_str() );

			gameplayBase->getAnimal()->event( Animal::FromFSLEnum( cosmosfsl->getInt("introAnim" ) ) );
			gameplayBase->getAnimal()->idle = true; /// <!>

			skipText->wEdit->setCaption( P3DScripts::p3dVars->getRoot()->getCString("flyinStart") );
			hintBox->setText(cosmosfsl->getClass("introAnimal")->getCString("i"));
			hintBox->open();

			VisualBoss::getSingletonPtr()->remove(this);
		}

		return false;
	}

	void FlyIn::end(void*) {
		if (anim)
		{
			SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
			sceneBase->getSceneManager()->destroyAnimationState(FLY_IN);
			anim = 0;

			sceneBase->setCamera( origCam );
			SoundBoss::getSingletonPtr()->setCamera(origCam);
			myCam->getParentSceneNode()->setVisible(false);
			VisualBoss::getSingletonPtr()->remove(this);
			gameplayBase->getAnimal()->idle = false; /// <!>
			acceptMouseEvents(false);
			acceptKeyEvents(false);
			hintBox->close(); /// does nothing if not already opened
			skipText->close();
		}
		if (myQueue)
		{
			myQueue->next();
			myQueue = 0;
		}
	}

	void FlyIn::onMouseDown() {

		/// left-click : stops intro
		/// right-click : fast-forward
		/// middle-click : pause
		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isButtonDown(HIDBoss::MOUSE_LEFT)) {
			control = FAST_FORWARD;
		}
		if (hid->isButtonDown(HIDBoss::MOUSE_RIGHT)) {
			end();
		}
		if (hid->isButtonDown(HIDBoss::MOUSE_MIDDLE)) {
			control = PAUSE;
		}
	}

	void FlyIn::onKeyEvent() {

		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isKeyDown(OIS::KC_LEFT)) {

			control = REWIND;

		} else if (hid->isKeyDown(OIS::KC_UP)) {

			control = RESET;
		}

		if (hid->isKeyDown(OIS::KC_ESCAPE)) {
			end();
		}
	}
}