// -----------------------------------
// P3DExtro - game extroduction sequence
// -----------------------------------

#pragma once
#ifndef P3DExtro_H
#define P3DExtro_H

#include <string>

#include <hid\HIDListener.h>
#include <OgreFrameListener.h>
#include <tween\TPlayed.h>

#include <map\Map.h>
#include <OgreAddOns.h>

namespace P3D {

	class TPlaylist;
	class GameplayCollection;
	class WiText;

	class P3DExtro : public Ogre::FrameListener, public HIDListener, public TPlayed, public Ogre::CompositorInstance::Listener {

	public:

		static const double CAMERA_ROT_SPEED;
		static const double EARTH_ROT_SPEED;
		static const double ENERGY_HEIGHT;
		static const double ENERGY_SCALE_TIME;
		static const double ENERGY_SCALE_DELAY;

		static bool ShowSuperEnding;

		P3DExtro();
		virtual ~P3DExtro();

		void begin();

		const double& getRadialDistance() const { return sampleDist; };
		void setRadialDistance(const double& value);
		const double& getRadialStrength() const { return sampleStrength; };
		void setRadialStrength(const double& value);
		const double& getBloomWeight() const { return blurWeight; };
		void setBloomWeight(const double& value);
		const double& getOriginalWeight() const { return originalImageWeight; };
		void setOriginalWeight(const double& value);

		virtual bool frameRenderingQueued(const Ogre::FrameEvent&);
		virtual void onKeyEvent();
		virtual void onMouseDown();
		virtual void playlistFinished(TPlaylist* list);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	private:

		static const int CREDIT_LIST_ID = 9876;
		static const int CAMERA_LIST_ID = 1987;
		static const int ANIMAL_LIST_ID = 2345;
		
		Ogre::GpuProgramParametersSharedPtr bloomParams;
		Ogre::GpuProgramParametersSharedPtr radialParams;
		double sampleStrength;
		double sampleDist;
		double blurWeight;
		double originalImageWeight;

		GameplayCollection* mapCollection;

		Ogre::SceneNode* earthRoot;
		Ogre::SceneNode* camera;	/// actual Ogre camera holder
		Ogre::SceneNode* cameraRot;	/// 'camera' node parent
		NodeWrapper cameraWrapper;

		MyGUI::VectorWidgetPtr _widgets;
		std::vector<WiText*> widgets;

		unsigned int currentCredit;
		void nextCredit(double extraDelay = 0.);

		typedef std::map<GameplayCollection*, NodeWrapper> AnimalCollections;
		AnimalCollections animalCollections;
		AnimalCollections::iterator currentAnimal;
		AnimalCollections::iterator oldestUnpausedAnimal;
		int runningAnimals;
		void nextAnimal(double extraDelay = 0.);

		bool notFinished;
	};

}

#endif // P3DExtro_H