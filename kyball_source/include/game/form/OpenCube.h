///////////
// OpenCube - a 'Form' descendant
///////////

#pragma once
#ifndef OpenCube_H
#define OpenCube_H

#include "Form.h"
#include "BallState.h"
#include "game\GameplayCollection.h"
#include "game\event\EventListener.h"
#include "tween\TPlayed.h"

#include <string>
#include <OgreColourValue.h>

namespace Ogre {
	class SceneNode;
	class Pass;
	class Entity;
}

namespace P3D {

	class EventManager;
	class ShrinkEvent;
	class GameplayBase;

	class OpenCube : public Form, public GameplayCollection, public EventListener, public TPlayed { /// <!!!> INHERITANCE ORDER IS IMPORTANT <!!!>

	private:

		Ogre::SceneNode* sideA[4];
		Ogre::SceneNode* sideB[4];
		Ogre::SceneNode* sideC[4];
		Ogre::SceneNode* sideD[4];

		Ogre::SceneNode* corners[8];
		unsigned int cornerCount;

		Ogre::SceneNode* edges[12];

		/// helping functions
		/// - build side edge
		Ogre::Entity* buildSideEdge(std::string, Ogre::SceneNode*, double, double, double);
		/// - build top/bottom edge + corner
		/// =>	name prefix
		///		node
		///		node rotation
		///		edge scale
		///		name postfix
		///		offset from origin
		/// <=	respective edge node
		Ogre::SceneNode* buildEdgeCorner(std::string&, Ogre::SceneNode*, const Ogre::Vector3& rot, double, const char*);

		bool dead;
		GameplayBase* base;
		Ogre::Pass* edgesPass;

		// misc
		void shrink(ShrinkEvent*);
		void colourEdges(const Ogre::ColourValue&, double dur);
		void shortCircuit();
		void levelWin();
		void powerUp();

		static const int SHRINKING_LIST_ID = 999; /// don't delete this Playlist when level is won!

	public:

		static const double EDGES_POWER_UP_SCALE;
		static const double EDGES_FADE_TIME;
		static const double LOSE_FADE_TIME;
		static const double WIN_FADE_TIME;

		static const Ogre::ColourValue EDGES_WIN_COLOR;

		// constructor
		// => base = GameplayBase* this stuff belongs to
		//	  zDepth = depth of entity
		//	  fix = added depth (split at the cube's 2 open ends); usually equal to a ball's radius!
		OpenCube(GameplayBase* base, double zDepth, double fix = BallState::BALL_RADIUS);
		virtual ~OpenCube();

		/// overrides
		virtual void incomingEvent(Event*);
		virtual void update();

	};
}

#endif