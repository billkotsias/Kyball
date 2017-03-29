/////////////
// BallEditor - create, edit and save ball structures
/////////////

#pragma once
#ifndef BallEditor_H
#define BallEditor_H

#include "hid\HIDListener.h"
#include "game\form\BallState.h"
#include <string>
#include <vector>

namespace P3D {

	class GameplayCollection;
	class GameplayBase;
	class Net;

	class BallEditor : public HIDListener {

	protected:

		GameplayBase* base;
		GameplayCollection* collection;	/// borrowed from "base"
		Net* net;
		Ogre::Entity* entity;
		Ogre::SceneNode* node;
		std::string text;				/// text to copy to clipboard

		int x,y,z;

		void updatePosition();
		void createBall(BallState::BallType);
		void save();

		static const std::string ballTypeStr[BallState::MAX_TYPES + BallState::MAX_BOBS];

	public:

		BallEditor(GameplayBase*);
		~BallEditor();

		// overrides
		virtual void onKeyEvent();

	};
}

#endif