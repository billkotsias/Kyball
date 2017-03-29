// -----------------------------------
// P3DIntro - game introduction screen
// -----------------------------------

#pragma once
#ifndef P3DIntro_H
#define P3DIntro_H

#include <string>

#include <hid\HIDListener.h>
#include <OgreFrameListener.h>
#include <tween\TPlayed.h>

namespace P3D {

	class TPlaylist;

	class P3DIntro : public Ogre::FrameListener, public HIDListener, public TPlayed {

	private:

		double fade;
		double fadeStay;

		typedef std::vector<Ogre::MaterialPtr> MaterialContainer;
		bool notFinished;
		std::vector<std::string> splashes;
		MaterialContainer materials;
		Ogre::OverlayContainer* panel;
		TPlaylist* list;

		Ogre::TextureUnitState* passTex;
		void setCurrentTextureAlpha(const Ogre::Real& value);
		const Ogre::Real& getCurrentTextureAlpha() const;
		Ogre::Real passTextAlpha;

	public:

		static const double FADE;
		static const double FADE_STAY;
		static const std::string SPLASH_RES_GROUP;

		P3DIntro();
		~P3DIntro();

		void begin(std::string resourceGroup = SPLASH_RES_GROUP, double fadeIn = FADE, double afterFadeStay = FADE_STAY);
		void nextSplash();
		void prematureSplashEnd();

		virtual bool frameRenderingQueued(const Ogre::FrameEvent&);
		virtual void onKeyEvent();
		virtual void onMouseDown();
		virtual void playlistFinished(TPlaylist*);

	}; // class P3DIntro

} // namespace

#endif // P3DIntro_H