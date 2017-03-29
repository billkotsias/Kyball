#include "StdAfx.h"
#include "visual\PreviewLevel.h"

#include "Queue.h"
#include "visual\VisualBoss.h"
#include "sound\SoundBoss.h"
#include <hid\HIDBoss.h>
#include "P3DWindow.h"
#include "SceneBaseCreator.h"

namespace P3D {

	PreviewLevel::PreviewLevel(const char* const _cam, double _time) : cam(_cam), time(_time), myCam(0) {
	}

	void PreviewLevel::start(void* queue) {
		myQueue = (Queue*)queue;

		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		Ogre::SceneManager* sceneMan = sceneBase->getSceneManager();
		try {
			myCam = sceneMan->getCamera(cam);
		} catch(...) {
			myQueue->next();
			return;
		}

		/// - set camera
		origCam = sceneBase->getCamera();
		myCam->setAspectRatio( origCam->getAspectRatio() );
		myCam->setNearClipDistance( origCam->getNearClipDistance() );
		myCam->setFarClipDistance( origCam->getFarClipDistance() );
		sceneBase->setCamera( myCam );
		SoundBoss::getSingletonPtr()->setCamera(myCam);
		myCam->getParentSceneNode()->setVisible(true);

		/// - start counting
		VisualBoss::getSingletonPtr()->insert(this);

		/// - listen to "skip"
		acceptMouseEvents(true);
	}

	PreviewLevel::~PreviewLevel() {
	}

	bool PreviewLevel::run(double timeToRun) {
		time -= timeToRun;
		if (time <= 0) {
			stop();
			return true; /// delete me
		}
		return false;
	}

	void PreviewLevel::stop() {
		if (!myCam) return;

		/// - restore camera
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		sceneBase->setCamera( origCam );
		SoundBoss::getSingletonPtr()->setCamera(origCam);
		myCam->getParentSceneNode()->setVisible(false);
		///VisualBoss::getSingletonPtr()->remove(this); /// automatically called on destruction

		/// - don't listen to "skip"
		acceptMouseEvents(false);

		myCam = 0;
		time = 0;
		myQueue->next();
	}

	void PreviewLevel::onMouseDown() {
		HIDBoss* hid = HIDBoss::getSingletonPtr();
		if (hid->isButtonDown(HIDBoss::MOUSE_LEFT)) {

			stop();
		}
	}
}