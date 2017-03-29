// ---------
// P3DConfig
// ---------

#pragma once
#ifndef P3DConfig_H
#define P3DConfig_H

#include "hid\HIDListener.h"
#include "GameQuality.h"

#include <vector>
#include <string>

namespace P3D {

	class SubConfig;
	class P3DConfig;

	// 
	class P3DConfigHelper : public HIDListener {

	private:

		P3DConfig* cfg;

	public:

		P3DConfigHelper(P3DConfig* _cfg) : HIDListener(true, true), cfg(_cfg) {};

		void onKeyEvent();
		void onMouseChange();
	};

	//

	class P3DConfig : public Ogre::FrameListener {

	private:

		bool doNotBegin;
		bool notFinished;

		Ogre::Root* ogreRoot;

		static unsigned int WIDTH;
		static unsigned int HEIGHT;
		static unsigned int DEPTH;
		static unsigned int FSAA;
		static double MONITOR_ASPECT_RATIO;

		/// available settings
		std::vector<unsigned int> fsaa;
		std::vector<DEVMODEA> modes;
		DEVMODEA currentMode;

		void subConfigClosed(void*);

		SubConfig* subConfig;

		friend class P3DWindow;
		friend class SceneBaseCreator;
		friend class P3DConfigHelper;	

		bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	public:

		static const std::string RESOURCES_FILE;
		static GameQuality::Option gameQuality;

		~P3DConfig();

		bool begin();
		void setMode(GameQuality::Option option);

		unsigned int getMaxFSAA();
		unsigned int getMedFSAA();
		unsigned int getMinFSAA();

		const DEVMODEA& getMaxMode();
		const DEVMODEA& getMedMode();
		const DEVMODEA& getMinMode();

		const DEVMODEA* getNearestMode(unsigned int x, unsigned int y, unsigned int bpp); /// <= 0 if doesn't exist (e.g there are no modes with given bpp)
	};

}

#endif