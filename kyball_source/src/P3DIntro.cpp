#include "StdAfx.h"
#include "P3DIntro.h"
#include "P3DWindow.h"
#include "P3DApp.h"

#include "hid\HIDBoss.h"
#include "tween\TPlayer.h"
#include "tween\TLinear.h"
#include "SceneBaseCreator.h"
#include "OgreBuilder.h"

#include <OgreResourceGroupManager.h>
#include <OgrePanelOverlayElement.h>
#include <algorithm>

#include <time.h>
#include <core\utils.h>

namespace P3D {

	const std::string P3DIntro::SPLASH_RES_GROUP = "BRANDING";
	const double P3DIntro::FADE = 1.0;
	const double P3DIntro::FADE_STAY = 1.5;

	P3DIntro::P3DIntro() : HIDListener(), notFinished(true) {

	}

	void P3DIntro::begin(std::string resourceGroup, double fadeIn, double afterFadeStay)
	{
		fade = fadeIn;
		fadeStay = afterFadeStay;

		// find & sort resources (i.e splash images)
		Ogre::ResourceGroupManager* resMan =  Ogre::ResourceGroupManager::getSingletonPtr();
		Ogre::FileInfoListPtr fileInfoList = resMan->listResourceFileInfo(resourceGroup);
		if (!fileInfoList->size()) return; /// no splash screens, joy!

		resMan->listResourceFileInfo(resourceGroup);
		for (unsigned int i = 0; i < fileInfoList->size(); ++i) {
			const Ogre::FileInfo& fileInfo = fileInfoList->at(i);
			//resMan->declareResource(fileInfo.basename, "Texture", resourceGroup); /// already initialized ALL
			splashes.push_back(fileInfo.basename);
		}
		//resMan->initialiseResourceGroup(resourceGroup);	// already initialized ALL
		//resMan->loadResourceGroup(resourceGroup);			// does nothing at this point
		std::sort(splashes.rbegin(), splashes.rend()); /// reverse sort cause they are in a vector (only pop-back)...

		// create Overlay stuff
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		sceneBase->createSceneBase();
		Ogre::OverlayManager* const overlayManager = Ogre::OverlayManager::getSingletonPtr();
		Ogre::Overlay* overlay = overlayManager->create("splashOverlay");
		panel = (Ogre::OverlayContainer*)( overlayManager->createOverlayElement("Panel", "panel") );
		panel->setMetricsMode(Ogre::GMM_RELATIVE_ASPECT_ADJUSTED); /// vertical resolution set to <10000> square units!
		panel->setHorizontalAlignment(Ogre::GHA_CENTER); /// this is, relative to the screen
		panel->setVerticalAlignment(Ogre::GVA_CENTER);
		overlay->add2D( panel );
		overlay->show();

		// create each splash ('Panel' material) separately
		Ogre::MaterialManager* materialManager = Ogre::MaterialManager::getSingletonPtr();
		for (unsigned int i = 0; i < splashes.size(); ++i)
		{
			const std::string& splash = splashes.at(i);

			Ogre::MaterialPtr material = materialManager->getByName(splash);
			if (material.isNull())
			{
				/// create specific material
				material = materialManager->create(splash, resourceGroup, false);
				Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
				passTex = pass->createTextureUnitState( splash );
				setCurrentTextureAlpha( 0. );
				material->setLightingEnabled(false);
				material->setDepthCheckEnabled(false);
				material->setDepthWriteEnabled(false);
				material->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
				material->setCullingMode(Ogre::CULL_NONE);
			}
			materials.push_back( material );
			panel->setMaterialName( material->getName() ); /// force texture preload/upload <NOW>
		}

		// begin fiesta
		MyGUI::Gui::getInstancePtr()->setVisiblePointer(false);
		nextSplash();
		Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
		ogreRoot->addFrameListener(this);
		acceptKeyEvents(true);
		acceptMouseEvents(true);
		ogreRoot->startRendering();
		// end fiesta
		ogreRoot->removeFrameListener(this);

		//
		//

		// destroy splash materials & overlay
		for (MaterialContainer::iterator it = materials.begin(); it != materials.end(); ++it) {
			Ogre::MaterialPtr material = *it;
			material->removeAllTechniques();
			materialManager->remove( material->getName() );
		}
		materials.clear();
		resMan->unloadResourceGroup(resourceGroup);

		overlay->remove2D( panel );
		overlayManager->destroyOverlayElement( panel );
		overlayManager->destroy( overlay );

		// destroy scene
		sceneBase->destroySceneBase();
	}

	void P3DIntro::nextSplash() {
		std::cout << "nextSplash=" << materials.size() << "\n";
		if (!materials.size()) {
			notFinished = false;
			return;
		}

		/// set material and dimensions
		Ogre::MaterialPtr material = materials.back();
		panel->setMaterialName( material->getName() );
		materials.pop_back();

		Ogre::TexturePtr textureRes = Ogre::TextureManager::getSingletonPtr()->getByName( splashes.back() );
		splashes.pop_back();
		const double texRatio = (double)textureRes->getWidth() / textureRes->getHeight();

		static const double screenHeight = 10000;
		double dimX = screenHeight * P3DWindow::getWindowRatio();
		double dimY = dimX / texRatio * P3DWindow::getPixelRatio();
		if (dimY > screenHeight) {
			dimX *= screenHeight / dimY;
			dimY = screenHeight;
		}
		panel->setDimensions( dimX, dimY );
		panel->setPosition( -0.5*dimX, -0.5*dimY );

		/// perform fade
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		passTex = pass->getTextureUnitState(0);
		setCurrentTextureAlpha(0);
		list = new TPlaylist();
		list->add(new TLinear<P3DIntro, Ogre::Real>(
			this, 1., fade, &P3DIntro::getCurrentTextureAlpha, &P3DIntro::setCurrentTextureAlpha)
			);
		list->add(new TLinear<P3DIntro, Ogre::Real>(
			this, 0., fade, &P3DIntro::getCurrentTextureAlpha, &P3DIntro::setCurrentTextureAlpha, fadeStay)
			);
		registerList( list );
	}

	void P3DIntro::prematureSplashEnd() {
		Ogre::Real temp = passTextAlpha;
		deleteTweens(); /// this nullifies passTextAlpha

		// TPlayer causes bug when ZERO time is passed as duration to a tween!
		if (temp < 1./P3DApp::CYCLES_PER_SEC)
		{
			nextSplash();
		} else
		{
			setCurrentTextureAlpha(temp);
			list = new TPlaylist();
			list->add(new TLinear<P3DIntro, Ogre::Real>(
				this, 0., passTextAlpha * fade, &P3DIntro::getCurrentTextureAlpha, &P3DIntro::setCurrentTextureAlpha)
				);
			registerList( list );
		}
	}

	void P3DIntro::playlistFinished(TPlaylist* list) {
		this->TPlayed::playlistFinished(list);
		nextSplash();
	}

	bool P3DIntro::frameRenderingQueued(const Ogre::FrameEvent& evt) {

		if (P3DWindow::getWindow()->isClosed()) return false; /// check if app was externally quit

		int frames = P3DApp::CYCLES_PER_SEC * evt.timeSinceLastFrame; /// loop will run at least once
		static const double frameTime = 1./P3DApp::CYCLES_PER_SEC;
		do {
			/// runners
			TPlayer::getSingletonPtr()->run(frameTime);
			HIDBoss::getSingletonPtr()->captureDevices();
		} while (--frames > 0);

		return notFinished;
	}

	void P3DIntro::onKeyEvent() {
		if (HIDBoss::getSingletonPtr()->isKeyDown()) prematureSplashEnd();
	}

	void P3DIntro::onMouseDown() {
		prematureSplashEnd();
	}

	void P3DIntro::setCurrentTextureAlpha(const Ogre::Real& value) {
		passTextAlpha = value;
		passTex->setAlphaOperation(Ogre::LBX_BLEND_TEXTURE_ALPHA, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, value);
	}
	const Ogre::Real& P3DIntro::getCurrentTextureAlpha() const {
		return passTextAlpha;
	}

	P3DIntro::~P3DIntro() {
		
	}
}