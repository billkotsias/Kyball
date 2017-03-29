#include "StdAfx.h"
#include "game\net\BasicNet.h"

#include "collision\CollisionObject.h"
#include "OgreAddOns.h"
#include "OgreBuilder.h"

#include <OgreSceneNode.h>

namespace P3D {

	BasicNet::BasicNet(double size, Ogre::SceneNode* node) : Net(size) {
		origin = node->createChildSceneNode();
		pivot = origin->createChildSceneNode();
	}

	BasicNet::~BasicNet() {
		OgreBuilder::destroySceneNode(pivot);
		OgreBuilder::destroySceneNode(origin);
	}

	void BasicNet::reset() {
		origin->setPosition(0,0,0);
		origin->setScale(1,1,1);
		origin->setOrientation(Ogre::Quaternion());

		if (!reserved.empty()) {
			std::cout << "BasicNet : Reserved Position not empty !!!!!!!!!!!!!!!!!!!!!!!!!####################################################\n";
		}
		reserved.clear();
	}

	NetNode BasicNet::reserveNode(const Ogre::Vector3& pos) {
		int x, y, z;
		getNearestNode(pos, x, y, z);
		NetNode netNode(x, y, z);

#ifdef _DEBUG
		reserved[netNode] = Ogre::Vector3(x,y,z);
#else
		reserved[netNode] = true;
#endif

		return netNode;
	}

	void BasicNet::reserveNode(const NetNode& netNode) {
#ifdef _DEBUG
		reserved[netNode] = netNode.split();
#else
		reserved[netNode] = true;
#endif
	}

	void BasicNet::releaseNode(const NetNode& netNode) {
		if (reserved.erase(netNode) == 0) {
			reserved.size();
		}
	}

	bool BasicNet::isNodeReserved(const NetNode& netNode) {

#ifdef _DEBUG
		std::map<NetNode, Ogre::Vector3>::iterator it = reserved.find(netNode);
#else
		std::map<NetNode, bool>::iterator it = reserved.find(netNode);
#endif

		if (it != reserved.end()) {
			//std::cout << "BasicNet : Node is UNAVAILABLE (RESERVED) !!!!!!!!!!!!\n";
			return true;
		}
		return false;
	}

	void BasicNet::pushNode(std::vector<std::pair<Ogre::Vector3, NetNode> >& alter, int x, int y, int z) {
		alter.push_back(std::pair<Ogre::Vector3, NetNode>(netNodeToWorld(x,y,z), NetNode(x,y,z)));
	}

	std::pair<Ogre::Vector3, NetNode> BasicNet::findNode(const Ogre::Vector3& pos, const CollisionObject* const obj) {

		int x,y,z;
		getNearestNode(pos, x, y, z);

		std::vector<std::pair<Ogre::Vector3, NetNode> > alter;
		alter.reserve(20);
		pushNode(alter, x,y,z);

		pushNode(alter, x+1,y,z);
		pushNode(alter, x,y+1,z);
		pushNode(alter, x,y,z+1);

		pushNode(alter, x-1,y,z);
		pushNode(alter, x,y-1,z);
		pushNode(alter, x,y,z-1);

		pushNode(alter, x+1,y+1,z);
		pushNode(alter, x+1,y-1,z);
		pushNode(alter, x-1,y+1,z);
		pushNode(alter, x-1,y-1,z);

		pushNode(alter, x,y+1,z+1);
		pushNode(alter, x,y+1,z-1);
		pushNode(alter, x,y-1,z+1);
		pushNode(alter, x,y-1,z-1);

		pushNode(alter, x+1,y,z+1);
		pushNode(alter, x+1,y,z-1);
		pushNode(alter, x-1,y,z+1);
		pushNode(alter, x-1,y,z-1);

#ifdef _DEBUG
		reserved;
#endif

		/// was: discard nodes that don't have the same distance from the 'CollisionObject' as the original pre-stuck position
		const double TOLERANCE = 0.05 * origin->_getDerivedScale().x; /// <NOTE> : <0.01> may by <LOW>; increase to <0.05> if there's a crash!
		double originalDistance = obj->distance(pos);

		std::vector<std::pair<Ogre::Vector3, NetNode>*> alterCleared;
		alterCleared.reserve(20);
		for (unsigned int i = 0; i < alter.size(); ++i) {
			double nodeDistance = obj->distance(alter[i].first);
			if ( abs( nodeDistance - originalDistance ) < TOLERANCE ) alterCleared.push_back(&alter[i]);
		}

		/// choose from cleared nodes the closest one to 'pos'
		int minPos = -1;
		double minDist = Infinite;

		for (int i = alterCleared.size() - 1; i >= 0; --i) {
			double distance = pos.distance(alterCleared[i]->first);
			if (distance < minDist) {
				/// node looks fine, but is it available???
				if (isNodeReserved(alterCleared[i]->second)) continue;
				minDist = distance;
				minPos = i;
			}
		}
		if (minPos != -1) {
			return *alterCleared[minPos];
		}

		/// last defence from crashing : get all nodes <!!!>
		for (int i = alter.size() - 1; i >= 0; --i) {
			double distance = pos.distance(alter[i].first);
			if (distance < minDist) {
				/// node looks fine, but is it available???
				if (isNodeReserved(alter[i].second)) continue;
				minDist = distance;
				minPos = i;
			}
		}
		return alter[minPos];
	}

}