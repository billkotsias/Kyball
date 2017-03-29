#include "StdAfx.h"
#include "game\net\CubeNet.h"

#include <math\Math.h>
#include "OgreAddOns.h"

namespace P3D {

	CubeNet::CubeNet(double size, Ogre::SceneNode* node) : BasicNet(size, node) {
	}

	Ogre::Vector3 CubeNet::netNodeToWorld(const int x, const int y, const int z) {
		pivot->setPosition(x, y + r, z);
		return OgreAddOns::getDerivedPosition(pivot);
	};

	/// get nearest net-node position to actual position 'pos'
	void CubeNet::getNearestNode(const Ogre::Vector3& worldPos, int &x, int &y, int &z) {
		/// reverse algorithm
		Ogre::Vector3 pos = OgreAddOns::worldToLocal(pivot, &worldPos); /// get <local> position

		/// <NOTE> : instead of <round>, we use <floor>!
		z = FANLib::Math::round( pos.z );
		y = FANLib::Math::round( pos.y - r );
		x = FANLib::Math::round( pos.x );
	}

	double CubeNet::calcZDiff() {
		return gridSize;
	}
}
