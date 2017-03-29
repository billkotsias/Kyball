#include "StdAfx.h"
#include "SceneBaseCreator.h"

#include "P3DApp.h"
#include "P3DConfig.h"
#include "P3DWindow.h"
#include "OgreAddOns.h"
#include "hid\HIDBoss.h"

#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreRenderWindow.h>

#include "core\Utils.h"

namespace P3D {

	const char* const SceneBaseCreator::SCENEMANAGER_NAME = "SceneBaseCreatorManager";

	SceneBaseCreator* SceneBaseCreator::instance = 0;	/// singleton instance not created yet

	SceneBaseCreator::SceneBaseCreator() : window(0), sceneManager(0), lastSceneManagerType(0), myGUI(0), myPlatform(0) {
		clearPointers();
	}

	SceneBaseCreator* SceneBaseCreator::getSingletonPtr() {
		if (!instance) instance = new SceneBaseCreator();
		return instance;
	}

	void SceneBaseCreator::destroy() {
		if (instance) delete instance;
	}

	SceneBaseCreator::~SceneBaseCreator() {
		instance = 0; /// putting this here allows someone to directly delete SceneBaseCreator*, bypassing the static 'destroy()' function (if friend class, that is)
		shutdown();
	}

	void SceneBaseCreator::clearPointers() {
		for (int i = 0; i < MAX_VIEWS; ++i) {
			camera[i] = 0;
			viewport[i] = 0;
			viewportZOrder[i] = 0;
		}
	}

	void SceneBaseCreator::init(Ogre::RenderWindow* win) {
		window = win;
		ogreRoot = Ogre::Root::getSingletonPtr();
	}

	void SceneBaseCreator::setCamera(Ogre::Camera* newCamera, unsigned char i)
	{
		currentlySetCamera[i] = newCamera;
		Ogre::Camera* cam = camera[i];
		newCamera->setAspectRatio(cam->getAspectRatio());
		//newCamera->setProjectionType(cam->getProjectionType());
		//newCamera->setLodCamera(cam->getLodCamera());
		//newCamera->setUseRenderingDistance(cam->getUseRenderingDistance());
		//newCamera->setCullingFrustum(cam->getCullingFrustum());
		copyCurrentCameras();
	}

	bool SceneBaseCreator::frameRenderingQueued(const Ogre::FrameEvent &)
	{
		copyCurrentCameras();
		return true;
	}
	void SceneBaseCreator::copyCurrentCameras()
	{
		for (int i = numViews - 1; i >= 0; --i)
		{
			Ogre::Camera* sourceCam = currentlySetCamera[i];
			if (!sourceCam) continue;
			if (sourceCam->getParentSceneNode()) sourceCam->getParentSceneNode()->_update(true, true);
			Ogre::Camera* viewCam = camera[i];
			//viewCam->setDirection( sourceCam->getRealDirection() );
			viewCam->setOrientation( sourceCam->getRealOrientation() );
			viewCam->setPosition( sourceCam->getRealPosition() );
			viewCam->setFOVy( sourceCam->getFOVy() );
			viewCam->setNearClipDistance( sourceCam->getNearClipDistance() );
			viewCam->setFarClipDistance( sourceCam->getFarClipDistance() );

			Ogre::SceneNode* autoTrackNode = sourceCam->getAutoTrackTarget();
			if (autoTrackNode) {
				Ogre::Vector3 lookAtPos = OgreAddOns::getDerivedPosition(autoTrackNode);
				viewCam->lookAt( lookAtPos );
			}
		}
	}

	void SceneBaseCreator::createSceneBase(unsigned char views, SceneBaseCreator::ViewAlign align, int order,
										   Ogre::SceneType newType, char* camName
										   ) {

		if (!window)			OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
								 "SceneBaseCreator has not been initialized",
								 "SceneBaseCreator");
		if (viewport[0])		OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
								 "Previously created 'SceneBase' has not been destroyed",
								 "SceneBaseCreator");
		if (views > MAX_VIEWS)	OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE,
								 "Too many views requested",
								 "SceneBaseCreator");

		numViews = views;

		/// the order of these lines is important
		/// 1 - create <SceneManager>; IF NEEDED!
		if (!sceneManager || lastSceneManagerType != newType) {
			shutdown(); /// <!!!>
			sceneManager = ogreRoot->createSceneManager(newType, SCENEMANAGER_NAME);

			if (P3DConfig::gameQuality >= GameQuality::ADVANCED)
			{
				/// just add shadows
				sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
				sceneManager->setShadowColour( Ogre::ColourValue(0.66, 0.66, 0.66) );
				sceneManager->setShadowFarDistance( 75 );
				sceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_RGB);
				sceneManager->setShadowTextureSize(2048);
				sceneManager->setShadowUseLightClipPlanes(false);
			}
		}

		/// - define screen aspect ratio
		double aspectRatio;
		if (window->isFullScreen()) {
			aspectRatio = P3DConfig::MONITOR_ASPECT_RATIO;
		} else {
			aspectRatio = P3DWindow::getWindowRatio();
		}

		// This is probably bullshit (to be found-out in Kyball2 - split-screen play)
		//switch (align) {
		//	case SceneBaseCreator::LEFT_TO_RIGHT:
		//		aspectRatio /= (double)numViews;
		//		break;
		//	case SceneBaseCreator::TOP_TO_BOTTOM:
		//		aspectRatio *= (double)numViews;
		//		break;
		//	case SceneBaseCreator::TILED:
		//		/// <TODO>
		//		break;
		//}

		/// 2 & 3 - create <Cameras> and <Viewports>
		for (int i = 0 ; i < numViews; ++i)
		{
			/// 2 - create <Camera>
			std::string cameraName(camName);
			cameraName += '#';
			cameraName += FANLib::Utils::toString(i+1);
			currentlySetCamera[i] = camera[i] = sceneManager->createCamera(cameraName); /// final camera name = 'camName' + '#i'
			/// - calculate camera's aspect ratio depending on monitor and full-screen settings!
			camera[i]->setAspectRatio(aspectRatio);
			//sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( camera[i] ); /// camera SceneNode parent

			/// 3 - create camera's <Viewport>
			viewportZOrder[i] = order++;
			viewport[i] = window->addViewport(camera[i], viewportZOrder[i]);
		}

		/// 4 - GUI
		if (!myGUI) {
			clock_t time = clock();

			myPlatform = new MyGUI::OgrePlatform();
			myPlatform->initialise(window, sceneManager, "General", "");
			MyGUI::LogManager::setSTDOutputEnabled(false);
			myGUI = new MyGUI::Gui();
			myGUI->initialise("core.xml", P3DApp::MYGUI_LOG_FILE);
			myGUI->setVisiblePointer(false);

			HIDBoss::getSingletonPtr()->clippingRectEnabled = true;
			HIDBoss::getSingletonPtr()->setClippingRect(0,0, window->getWidth(),window->getHeight());
			HIDBoss::getSingletonPtr()->setMousePosition(window->getWidth()/2 - 1, window->getHeight()/2 - 1);

			time = clock() - time;
			std::cout << "MYGUI reinit time taken(!)" << FANLib::Utils::toString( (float)(time) / CLOCKS_PER_SEC ) << "!!!\n";
		}

		lastSceneManagerType = newType; /// last, if some dude ever wants to check... that is never till I die <!!!>

		//for (int i = numViews - 1; i >= 0; --i) {
		//	currentlySetCamera[i] = 0;
		//}
		ogreRoot->addFrameListener(this); /// copy cameras around
	}

	void SceneBaseCreator::destroySceneBase() {
		if (!(viewport[0])) return; /// scene base already destroyed

		/// 4 - GUI
		if (myGUI) myGUI->destroyAllChildWidget();

		/// extra - destroy all scene entities
		try {
			sceneManager->clearScene();
		} catch(...) { std::cout << "OgreSceneManager::clearScene exception error"; };

		/// 3 & 2 - destroy <Cameras> and <Viewports>
		for (int i = numViews - 1; i >= 0; --i) {

			/// 3 - destroy <Viewport>
			window->removeViewport(viewportZOrder[i]);

			/// 2 - destroy <Camera>
			//Ogre::SceneNode* parentNode = camera[i]->getParentSceneNode();
			//if (parentNode) {
			//	parentNode->detachAllObjects();
			//	sceneManager->destroySceneNode(parentNode);
			//}
			sceneManager->destroyCamera(camera[i]);
		}

		ogreRoot->removeFrameListener(this); /// copy cameras around

		// 1 - destroy <SceneManager>
		//ogreRoot->destroySceneManager(sceneManager);
		//sceneManager = 0;

		clearPointers();
	}

	void SceneBaseCreator::shutdown()
	{
		destroySceneBase();

		if (myGUI) {
			myGUI->shutdown();
			delete myGUI;
			myGUI = 0;
		}
		if (myPlatform) {
			myPlatform->shutdown();
			delete myPlatform;
			myPlatform = 0;
		}

		if (sceneManager) ogreRoot->destroySceneManager(sceneManager); /// destroy previous
		sceneManager = 0;
	}
}