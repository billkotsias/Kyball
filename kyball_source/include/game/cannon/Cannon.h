// ------
// Cannon
// ------

#pragma once
#ifndef Cannon_H
#define Cannon_H

#include "hid\HIDListener.h"
#include <OgreMatrix3.h>
#include "game\form\BallState.h"
#include "tween\TPlayed.h" /// for the tube's shake

#include <logic\Enabled.h>
#include <string>

namespace Ogre {
	class SceneNode;
	class Entity;
	class Pass;
	class Billboard;
	class BillboardSet;
}

namespace P3D {

	class BallAI;
	class BallCreator;
	class Aim;
	class CannonSwarm;
	class GameplayBase;
	class GameplayObject;
	class Camera;
	class EventManager;
	class Sound;
	class ParticleSystem;

	template<class Class> class CallEvery;

	class Cannon : public HIDListener, public TPlayed, public FANLib::Enabled {
	private:

		Ogre::Entity* cannon;
		Ogre::SceneNode* cannonNode;

		Ogre::Entity* tube;
		Ogre::SceneNode* tubeNode;
		Ogre::Pass* tubePass;

		double horizontalRotation;
		double verticalRotation;
		Ogre::Matrix3 rotMatrix;

		double maxHorizontalRotation; /// lower on 1st levels, so that you don't get really lost from the beginning
		double maxVerticalRotation;

		P3D::Camera* camera;	/// set additional orientation to this camera
		EventManager* eMan;		/// send events

		Ogre::Camera* cannonViewCamera; /// 1st-person view

		double timeBetweenShots;
		double timeToNextShot;

		GameplayObject* nextBall;
		BallCreator* ballCreator;
		Aim* aim;
		Ogre::BillboardSet* bbset;	/// BBSet that is set after the ball is shot!

		void createNextBall();

		// visual elements
		/// - swarms
		std::string swarmName;
		CannonSwarm* swarm[2];
		int currentSwarm;
		Ogre::Vector3 startPos;	/// cached
		Ogre::Vector3 endPos;	/// cached

		/// - animations
		void fadeAll();
		void fadeinSwarm();
		Ogre::Billboard* nextBallBillboard;
		const Ogre::Vector2& getBBDim() const;	/// } <hack> Ogre::Billboard's set/get dimensions
		void setBBDim(const Ogre::Vector2&);	/// }
		static const int nextBallFadeID = 1;

		/// <BallState::BallType> to <Ogre::ColourValue> conversion table
		static const std::vector<Ogre::ColourValue> ballColours;

		Sound* fireworksSound;
		void stopFireworks(bool lastToo);
		ParticleSystem* particleSystem; /// ...cause forward declaration of typedefs is MESSY

	public:

		static const double FADEOUT_TUBE;
		static const double START_SCALE;
		static const double END_SCALE;
		static const double CANNON_HEIGHT;
		static const double FADEIN_SWARM;
		static const double GOTO_SWARM;
		static const double FADEOUT_SWARM; /// equals "FADEIN_TUBE" <!>
		static const double TOTAL_SWARM;

		static const double MAX_HORIZONTAL_ROTATION;
		static const double MAX_VERTICAL_ROTATION;

		static const double AIM_SPEED;

		// constructor
		// => node of whole cannon
		//	  tube subnode
		Cannon(Ogre::SceneNode*, Ogre::SceneNode*);
		virtual ~Cannon();

		// => GameplayBase
		void init(GameplayBase*); /// must be called <prior> to use
		GameplayBase* base;
		void reset(); /// probably essential for a 'GameplayBase' component; MUST BE CALLED <PRIOR> TO USAGE

		// initialization members; these MUST be attached before Cannon is put into play!
		void attachBallCreator(BallCreator*);
		void attachAim(Aim*);
		void setTimeBetweenShots(double);
		void setMaximumDeviation(double deviation); /// "maximum orientation"

		// playtime functions
		void orientCannon();	/// update 'rotMatrix' and orient tube
		void resetOrientation();
		void run(double);		/// run for a portion of time
		void runAim(int);		/// run 'Aim' this number of cycles
		void calcFutureBall();	/// now it's time to calculate the colour of nextball

		void setCannonView(bool);	/// enable 1st person view?

		// used by 'BallEditor'
		BallAI* getNextBallAI();

		// overrides
		virtual void onMouseMove();
		virtual void onMouseDown();
		virtual void onMouseChange();

		virtual void playlistFinished(TPlaylist*); /// from 'TPlayed' <!>

		void playFireworks();
		void playFireworksSound(void*);

		virtual void disable(FANLib::DisableID id);
	};

}

#endif