// ----------------------------------------------------------------------------------------
// SceneBaseCreator - Generic interface for creating/destroying the base for any Ogre Scene (singleton)
// ----------------------------------------------------------------------------------------

#pragma once
#ifndef SceneBaseCreator_H
#define SceneBaseCreator_H

namespace MyGUI {
	class Gui;
	class OgrePlatform;
}

namespace Ogre {
	class Root;
	class RenderWindow;
	class Camera;
	class Viewport;
}

namespace P3D {

	class SceneBaseCreator : public Ogre::FrameListener {

		friend class CosmosCreator;
		friend class LevelBuilder;
		friend class P3DExtro;

	public:

		static const char* const SCENEMANAGER_NAME;

		/// ways to align multiple viewports
		enum ViewAlign {
			LEFT_TO_RIGHT,
			TOP_TO_BOTTOM,
			TILED,			/// TODO : must determine if num of views is even or odd
		};

		static const int MAX_VIEWS = 2; /// might change if I am to support up to 4 players on screen. Must implement ViewAlign::TILED first.

		/// singleton logic
		static SceneBaseCreator* getSingletonPtr();	/// create at 1st call
		static void destroy();						/// call destructor statically

		// generic Ogre Scene interface
		// - initialize (essential)
		/// =>	win = Ogre RenderWindow
		void init(Ogre::RenderWindow*);

		// - create all basic stuff for an Ogre scene
		///	=>	views	= number of viewports to create; default = 1
		///		align	= viewport alignment; default = LEFT_TO_RIGHT
		///		order	= z order of the 1st viewport; descending for each subsequent viewport
		///		type	= base SceneManager type
		///		camName	= base camera name (<#x> added at the end of each camera name, starting with <x = 1>)
		/// <NOTE> : if the new SceneManager is of the same type as the last one created, it won't get recreated !! Saves much re-initialization and MyGUI loading !!
		void createSceneBase(unsigned char = 1, ViewAlign = LEFT_TO_RIGHT, int = 0,
							 Ogre::SceneType = Ogre::ST_GENERIC,
							 char* = "BaseCam"
							 );
		int lastSceneManagerType;

		// - destroy all basic stuff (inc. user created scene entities)
		/// <=	destroys all stuff created by previous function
		void destroySceneBase();
		void shutdown();

		// Ogre Scene base publicly available (getters for security)
		//Ogre::RenderWindow*	getWindow()			{ return window; }; // get it from P3DWindow, that's what it's there for
		Ogre::SceneManager*	getSceneManager() { return sceneManager; };

		int getViewsNum() { return numViews; };
		Ogre::Camera* getCamera	(unsigned char i = 0) { return currentlySetCamera[i]; };
		void setCamera (Ogre::Camera*, unsigned char i = 0);
		//int getViewportZOrder (unsigned char i = 0) { return viewportZOrder[i]; };

		// GUI
		MyGUI::Gui* getMyGUI() { return myGUI; };

	private:

		/// singleton : private constructor to prevent multiple instance creation
		SceneBaseCreator();
		static SceneBaseCreator* instance;
		~SceneBaseCreator();

		void clearPointers();

		Ogre::Root*			ogreRoot;
		Ogre::RenderWindow*	window;
		Ogre::SceneManager*	sceneManager;

		int					numViews;
		Ogre::Camera*		camera[MAX_VIEWS];
		Ogre::Camera*		currentlySetCamera[MAX_VIEWS];
		Ogre::Viewport*		viewport[MAX_VIEWS];
		int					viewportZOrder[MAX_VIEWS];

		MyGUI::Gui* myGUI;
		MyGUI::OgrePlatform* myPlatform;

		Ogre::Viewport* getViewport (unsigned char i = 0) { return viewport[i]; };

		void copyCurrentCameras();
		virtual bool frameRenderingQueued(const Ogre::FrameEvent&);
	};

}

#endif