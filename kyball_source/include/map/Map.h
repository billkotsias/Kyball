// ---------------------------
// Map - cosmos selection menu
// ---------------------------

#pragma once
#ifndef Map_H
#define Map_H

#include "menu\SubMainOptions.h"
#include "map\MapOptions.h"

#include "hid\HIDListener.h"
#include "tween\TPlayed.h"
#include "OgreAddOns.h"

#include <vector>

namespace P3D {

	class Player;
	class GameplayCollection;
	class SubMap;
	class HintBox;
	class SubBlack;
	class MenuBox;

	class MapEntity : public TPlayed {

	private:

		MapOptions::CosmosID id;
		Ogre::SceneNode* camera;
		Ogre::SceneNode* center;

		double scale;

	public:

		/// constructor
		MapEntity() : id(MapOptions::UNDEFINED), camera(0), center(0) {}; /// target(0),
		MapEntity(MapOptions::CosmosID i, Ogre::SceneNode* cam, Ogre::SceneNode* cen) : id(i), camera(cam), center(cen) {
			scale = center->getScale().x;
		};
		virtual ~MapEntity();

		MapOptions::CosmosID getID() { return id; };
		const Ogre::SceneNode* const getCamera() { return camera; };
		Ogre::SceneNode* getCenter() { return center; };

		void setScale(const double&);
		const double& getScale() const { return scale; };
	};


	//


	class Map : public Ogre::FrameListener, public HIDListener, public TPlayed {

	private:

		bool notFinished;
		bool zoomedIn;
		bool zoomingIn;
		bool fadingIn;
		double minimumDistance;
		MapEntity* selectedEntity;
		MapOptions mapOptions;
		bool hacked;

		GameplayCollection* mapCollection;

		Ogre::Vector2 rot;
		double time;

		Ogre::SceneNode* camera;	/// actual Ogre camera holder
		Ogre::SceneNode* cameraRot;	/// 'camera' node parent
		NodeWrapper cameraRotWrapper;
		//Ogre::SceneNode* cameraTarget;

		Ogre::Vector3 orbitPos;			/// save camera position prior to zoom-in
		Ogre::Quaternion orbitOrient;	/// save camera orientation prior to zoom-in
		double oriFactor;				/// helper for orientation setting (hack!)
		Ogre::Quaternion destOri;		/// destination orientation

		Ogre::SceneNode* mouseRot;
		Ogre::SceneNode* mouse;
		Ogre::BillboardSet* mBBSet;

		Ogre::SceneNode* earth;

		Ogre::SceneNode* earthRoot;
		NodeWrapper earthWrapper;

		std::vector<MapEntity> entities;
		std::vector<MapEntity*> activeEntities;
		MapEntity unselectedEntity;
		bool setActiveEntities();		/// THIS FUNCTION CHECKS IF THE WHOLE GAME HAS JUST ACTUALLY <FINISHED!!!!!!!!!>
		void setAllActiveEntities();	/// this, after the game has finished

		void followMouse();
		void selectEntity(MapEntity*);

		/// additional demo stuff
		std::vector<NodeWrapper> demoBalls; /// <useless> : should be 'SceneNode'
		Ogre::SceneNode* base;
		GameplayCollection* demoCollection;
		Ogre::MaterialPtr demoBallsMat;
		void destroyDemo();

		///
		std::deque<MenuBox*> levelBoxes;
		void createLevelSelection();
		void destroyLevelSelection();
		void levelBoxClicked(void*);
		void levelBoxClosed(void*);

	public:

		/// main
		static const double ZOOM_TIME;
		static const double FACTOR_END;
		static const double YROT_MAX;
		static const int ZOOM_OUT_ID;
		static const int ZOOM_IN_ID;
		static const int FADE_IN_ID;
		static const double RADIUS;

		/// demo
		static const double DAY_TIME;	/// time for a 360 rotation
		static const double POLAR_TIME;	/// time to go from pole-to-pole-to-pole
		static const double FULL_ROT;	/// a full rotation (360)
		static const double SCALE_TIME;
		static const int DEMO_OUT_ID;

		Map();
		~Map();

		Player* player;	/// current player
		HintBox* hintBox;
		SubMap* mapText;
		SubBlack* subBlack;

		// load, create scene base and wait
		void init();

		// demo
		void showDemo();					/// set initial tweens
		void runDemoFrame(double _time);	/// run a single demo frame
		void hideDemo();					/// unset initial tweens

		// show cosmos-selection map
		// - takes over as frame listener
		// - hides demo
		/* => mode = map mode */;
		/*		ADVENTURE	: visible all except hidden, selectable only of Player's current chapter		*/;
		/*		VERSUS		: visible all except hidden, selectable all that all "Players" have visited		*/;
		/*		SURVIVAL	: visible all except hidden, selectable all that Player has finished			*/;
		/*	  player = pointer to Player */;
		MapOptions show(const SubMainOptions::FinalChoice&, Player*);

		void gameOn(void*);

		// unload
		void unload();

		const std::string& getHintBoxStr();

		// overrides
		virtual bool frameRenderingQueued(const Ogre::FrameEvent&);
		virtual void onMouseMove();
		virtual void onMouseChange();
		virtual void onKeyEvent();
		virtual void playlistFinished(TPlaylist* list);

		void setCamPos(const Ogre::Vector3&);
		const Ogre::Vector3& getCamPos() const;

		void setOriFactor(const double& f);
		const double& getOriFactor() const { return oriFactor; };

		void setCamOri(const Ogre::Quaternion&);
		const Ogre::Quaternion& getCamOri() const;
	};
}

#endif