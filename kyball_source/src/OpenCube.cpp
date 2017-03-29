#include "StdAfx.h"
#include "game\form\OpenCube.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include "collision\Polygon.h"
#include "collision\Complex.h"
#include "game\GameplayBase.h"
#include "game\ai\AI.h"
#include "game\event\ShrinkEvent.h"

#include "tween\TLinear.h"
#include "tween\TCubeFX.h"
#include "tween\TPlayer.h"

#include "OgreAddOns.h"
#include "BurstBoss.h"

#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include <core\Utils.h>

namespace P3D {

	const double OpenCube::EDGES_POWER_UP_SCALE = 10.;
	const double OpenCube::EDGES_FADE_TIME = 0.66;
	const double OpenCube::LOSE_FADE_TIME = 2.0;
	const double OpenCube::WIN_FADE_TIME = 1.7;

	const Ogre::ColourValue OpenCube::EDGES_WIN_COLOR = Ogre::ColourValue(0.149,1,0.706);

	OpenCube::OpenCube(GameplayBase* base, double z, double fix) : dead(false), cornerCount(0) {

		this->base = base;
		std::string name = std::string("OpenCube") + base->getID();	/// name to give to the created entity
		std::string material = base->getCosmos() + "_cube";			/// name of material to use
		EventManager* eMan = base->getEventManager();				/// EventManager to listen to "shrink" events
		double x = base->getCubeSize().x;
		double y = base->getCubeSize().y;

		watchEvent(eMan, Event::SHRINK_LEVEL); /// will be unregistered in destructor
		watchEvent(eMan, Event::SHRINK_IN_2);
		watchEvent(eMan, Event::SHRINK_IN_1);
		watchEvent(eMan, Event::LOST);
		watchEvent(eMan, Event::WON);
		watchEvent(eMan, Event::COSMOS_FINISHED);

		static const double X = 0.5;
		static const double YT = 1.;		/// y-top
		static const double YB = -0.001;	/// y-bottom
		static const double Z1 = 1.;
		static const double Z2 = 2.;

		double zScale = z + fix * 2;
		pivot->translate(0,0,-fix);
		pivot->setScale(x, y, zScale);

		/// build <collisionObject>
		Ogre::SceneNode* p0 = newSceneNode(pivot, &Ogre::Vector3(X, YB, -Z2));
		Ogre::SceneNode* p1 = newSceneNode(pivot, &Ogre::Vector3(-X, YB, -Z2));
		Ogre::SceneNode* p2 = newSceneNode(pivot, &Ogre::Vector3(-X, YT, -Z2));
		Ogre::SceneNode* p3 = newSceneNode(pivot, &Ogre::Vector3(X, YT, -Z2));
		Ogre::SceneNode* p4 = newSceneNode(pivot, &Ogre::Vector3(-X, YB, Z2));
		Ogre::SceneNode* p5 = newSceneNode(pivot, &Ogre::Vector3(X, YB, Z2));
		Ogre::SceneNode* p6 = newSceneNode(pivot, &Ogre::Vector3(X, YT, Z2));
		Ogre::SceneNode* p7 = newSceneNode(pivot, &Ogre::Vector3(-X, YT, Z2));

		sideA[0] = p0; /// Left
		sideA[1] = p5;
		sideA[2] = p6;
		sideA[3] = p3;

		sideB[0] = p1; /// Right
		sideB[1] = p2;
		sideB[2] = p7;	
		sideB[3] = p4;

		sideC[0] = p2; /// Top
		sideC[1] = p7;
		sideC[2] = p6;
		sideC[3] = p3;

		sideD[0] = p0; /// Bottom
		sideD[1] = p5;
		sideD[2] = p4;
		sideD[3] = p1;

		Complex* complex = new Complex();
		complex->addObject(new Polygon(4, sideA));
		complex->addObject(new Polygon(4, sideB));
		complex->addObject(new Polygon(4, sideC));
		complex->addObject(new Polygon(4, sideD));
		collisionObject = complex;


		/// build <MovableObject>
		/// - walls
		Ogre::SceneNode* node1 = newSceneNode(pivot, &Ogre::Vector3(X, 0, 0));
		Ogre::SceneNode* node2 = newSceneNode(pivot, &Ogre::Vector3(0, YT, 0));
		Ogre::SceneNode* node3 = newSceneNode(pivot, &Ogre::Vector3(-X, 0, 0));

		std::string resourceName = "cube";
		storeMovable( OgreBuilder::createEntity(name + "_1", resourceName + "1.mesh", false, material, node1) );
		storeMovable( OgreBuilder::createEntity(name + "_2", resourceName + "2.mesh", false, material, node2) );
		storeMovable( OgreBuilder::createEntity(name + "_3", resourceName + "3.mesh", false, material, node3) );

		/// - side (z) edges
		edges[0] = newSceneNode(pivot, &Ogre::Vector3(X, YB, 0));
		edges[1] = newSceneNode(pivot, &Ogre::Vector3(-X, YB, 0));
		edges[2] = newSceneNode(pivot, &Ogre::Vector3(-X, YT, 0));
		edges[3] = newSceneNode(pivot, &Ogre::Vector3(X, YT, 0));
		buildSideEdge(name + "_e0", edges[0], x, y, zScale);
		buildSideEdge(name + "_e1", edges[1], x, y, zScale);
		buildSideEdge(name + "_e2", edges[2], x, y, zScale);
		Ogre::Entity* edgeEnt = buildSideEdge(name + "_e3", edges[3], x, y, zScale); /// used to get the 'cube_edge' material!

		edgesPass = edgeEnt->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0); /// <!!!>
		edgesPass->setSelfIllumination(Ogre::ColourValue(1,1,1));

		/// - front/back edges + corners
		Ogre::SceneNode* p8 = newSceneNode(pivot, &Ogre::Vector3(X, YB, 0));
		Ogre::SceneNode* p9 = newSceneNode(pivot, &Ogre::Vector3(-X, YB, 0));
		Ogre::SceneNode* p10 = newSceneNode(pivot, &Ogre::Vector3(-X, YT, 0));
		Ogre::SceneNode* p11 = newSceneNode(pivot, &Ogre::Vector3(X, YT, 0));
		Ogre::SceneNode* p12 = newSceneNode(pivot, &Ogre::Vector3(-X, YB, Z1));
		Ogre::SceneNode* p13 = newSceneNode(pivot, &Ogre::Vector3(X, YB, Z1));
		Ogre::SceneNode* p14 = newSceneNode(pivot, &Ogre::Vector3(X, YT, Z1));
		Ogre::SceneNode* p15 = newSceneNode(pivot, &Ogre::Vector3(-X, YT, Z1));

		edges[4] = buildEdgeCorner(name, p14, Ogre::Vector3(0,180,0), x, "0");
		edges[5] = buildEdgeCorner(name, p15, Ogre::Vector3(0,180,270), y, "1");
		edges[6] = buildEdgeCorner(name, p12, Ogre::Vector3(0,180,180), x, "2");
		edges[7] = buildEdgeCorner(name, p13, Ogre::Vector3(0,180,90), y, "3");
		edges[8] = buildEdgeCorner(name, p10, Ogre::Vector3(0,0,0), x, "4");
		edges[9] = buildEdgeCorner(name, p11, Ogre::Vector3(0,0,270), y, "5");
		edges[10] = buildEdgeCorner(name, p8, Ogre::Vector3(0,0,180), x, "6");
		edges[11] = buildEdgeCorner(name, p9, Ogre::Vector3(0,0,90), y, "7");
	}

	Ogre::Entity* OpenCube::buildSideEdge(std::string name, Ogre::SceneNode* node, double x, double y, double zScale) {
		node->setScale(1/x, 1/y, 1);
		Ogre::Entity* entity = OgreBuilder::createEntity(name, "cube_edge_z.mesh", false, "", node);
		storeMovable(entity);
		return entity;
	}

	Ogre::SceneNode* OpenCube::buildEdgeCorner(std::string& name, Ogre::SceneNode* p, const Ogre::Vector3& rot, double scale, const char* postFix)
	{
		corners[cornerCount++] = p;

		/// scaling
		OgreAddOns::removeParentScale(p);

		/// rotation
		static Ogre::Matrix3 rotMatrix;
		static Ogre::Quaternion quat;
		rotMatrix.FromEulerAnglesYXZ(Ogre::Degree(rot.y), Ogre::Degree(rot.x), Ogre::Degree(rot.z));
		quat.FromRotationMatrix(rotMatrix);
		p->rotate(quat);

		/// - corner
		storeMovable( OgreBuilder::createEntity((name + "_corner") + postFix, "cube_corner.mesh", false, "", p) );
		/// - edge
		Ogre::SceneNode* pE = newSceneNode(p, &Ogre::Vector3::ZERO);
		pE->setScale(scale,1,1);
		storeMovable( OgreBuilder::createEntity((name + "_edge") + postFix, "cube_edge.mesh", false, "", pE) );
		return pE;
	}

	OpenCube::~OpenCube() {
		edgesPass->setSelfIllumination(Ogre::ColourValue(1,1,1));
		deleteTweens();
	}

	void OpenCube::update() {
		/// NOTE : CUBE CORNERS MUST BE UNAFFECTED BY DIRECT-PARENT'S SCALING!!! But MUST be affected by global scaling!!!
		OgreAddOns::removeParentScale((Ogre::Node**)corners, cornerCount); // BUG : When cube becomes zero size
	}

	void OpenCube::incomingEvent(Event* e) {
		
		switch (e->getType()) {

			case Event::SHRINK_LEVEL:
				shrink((ShrinkEvent*)e);
				break;

			case Event::SHRINK_IN_2:
				colourEdges(Ogre::ColourValue(1,0.5,0), EDGES_FADE_TIME);
				break;

			case Event::SHRINK_IN_1:
				colourEdges(Ogre::ColourValue(1,0.10,0.20), EDGES_FADE_TIME);
				break;

			case Event::WON:
				levelWin();
				break;

			case Event::LOST:
				shortCircuit();
				break;

			case Event::COSMOS_FINISHED:
				powerUp();
				break;
		}
	}

	void OpenCube::levelWin() {
		/// delete all tweens except shrinking one
		//deleteTweens();
		TPlayer* player = TPlayer::getSingletonPtr();
		for (std::set<TPlaylist*>::iterator it = lists.begin(); it != lists.end(); ) {
			TPlaylist* list = *(it++); /// <NOTE>
			if (list->id != SHRINKING_LIST_ID) {
				lists.erase( list );
				player->deletePlaylist( list );
			}
		}
		/// proceed as before
		colourEdges(EDGES_WIN_COLOR, WIN_FADE_TIME);
		dead = true;
		/// particles
		for (int i = cornerCount - 1; i>= 0; --i) {
			base->getBurstBoss()->newBurst(BurstBoss::CUBE_WIN, corners[i]);
		}
	}

	void OpenCube::powerUp() {
		/// edges 0-3 are side ones, 4-11 are "z" ones
		for (int i = 0; i < 12; ++i) {
			Ogre::Vector3 finalScale = edges[i]->getScale();
			finalScale.y *= EDGES_POWER_UP_SCALE;
			if (i < 4) {
				finalScale.x *= EDGES_POWER_UP_SCALE; /// side edges
			} else {
				finalScale.z *= EDGES_POWER_UP_SCALE; /// top/bottom edges
			}
			registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
				edges[i], finalScale, WIN_FADE_TIME * 2., &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
				);
		}
		/// particles (super)
		for (int i = cornerCount - 1; i>= 0; --i) {
			Ogre::Quaternion rotation = (i == 2 || i == 6) ? Ogre::Quaternion( Ogre::Radian(Ogre::Degree(180)), Ogre::Vector3(0,0,1) ) : Ogre::Quaternion();
			base->getBurstBoss()->newBurst(BurstBoss::COSMOS_WIN, corners[i], Ogre::Vector3::ZERO, rotation);
		}
	}

	void OpenCube::colourEdges(const Ogre::ColourValue& colour, double dur) {
		if (dead) return;
		registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			edgesPass, colour, dur, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
	}

	void OpenCube::shortCircuit() {
		dead = true;
		deleteTweens();
		registerTween(new TCubeFX<Ogre::Pass, Ogre::ColourValue>(
			edgesPass, Ogre::ColourValue(0,0,0), LOSE_FADE_TIME, 4., &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
	}

	void OpenCube::shrink(P3D::ShrinkEvent *ev) {

		double time = ev->getTime();
		double shrink = ev->getShrinkage();
		Ogre::Vector3 endScale = pivot->getScale();
		endScale.z -= shrink;
		if (endScale.z <= 0.1) endScale.z = 0.5; // BUG FIX!
		//std::cout << std::string("endScale:") + FANLib::Utils::toString(endScale.z) + "\n";

		// TODO : keep list ID and DON'T delete!!!
		registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			pivot, endScale, time, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			)->id = SHRINKING_LIST_ID;

		colourEdges(Ogre::ColourValue(1,1,1), EDGES_FADE_TIME);
	}
}