#include "StdAfx.h"
#include "game\net\Net.h"

#include <utility>

namespace P3D {

	// NetNode

	NetNode::NetNode() {
		_id = -1; /// invalid 'id' = "not initialized"
	}

	NetNode::NetNode(int x, int y, int z) {

		unsigned int _x = x << (intBits - shift);
		unsigned int _y = y << (intBits - shift);
		unsigned int _z = z << (intBits - shift);

		_id = (_x >> (intBits - shift)) + (_y >> (intBits - shift * 2)) + (_z >> (intBits - shift * 3));
	}

	Ogre::Vector3 NetNode::split() const {
		int x = _id << (intBits - shift);
		x = x >> (intBits - shift);

		int y = _id << (intBits - shift * 2);
		y = y >> (intBits - shift);

		int z = _id << (intBits - shift * 3);
		z = z >> (intBits - shift);

		return Ogre::Vector3(x,y,z);
	}

	// Net

	Net* Net::init() {
		zDiff = calcZDiff();
		return this;
	}

	std::pair<Ogre::Vector3, NetNode> Net::findAndReserveNode(const Ogre::Vector3& pos, const CollisionObject* const obj) {

		std::pair<Ogre::Vector3, NetNode> pair = findNode(pos, obj);
		reserveNode(pair.second);
		return pair;
	}
}