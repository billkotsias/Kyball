#include "StdAfx.h"
#include "game\net\HexNet.h"

#include <math\Math.h>
#include "OgreAddOns.h"

namespace P3D {

	HexNet::HexNet(double size, Ogre::SceneNode* node) : BasicNet(size, node) {
	}

	Ogre::Vector3 HexNet::netNodeToWorld(const int x, const int y, const int z) {
		pivot->setPosition(
			r * (x * 2 + ( (y & 1) ^ (z & 1) )),
			r * (y * sqrt(3.) + sqrt(3.) / 3. * (z & 1)),
			r * (z * sqrt(8./3.))
			);
		return OgreAddOns::getDerivedPosition(pivot);
	};

	/// get nearest net-node position to actual position 'pos'
	void HexNet::getNearestNode(const Ogre::Vector3& worldPos, int &x, int &y, int &z) {
		/// reverse algorithm
		Ogre::Vector3 pos = OgreAddOns::worldToLocal(pivot, &worldPos); /// get <local> position

		z = FANLib::Math::round( pos.z / (r * sqrt(8./3.)) );
		y = FANLib::Math::round( (pos.y - sqrt(3.)/3. * r * (z & 1)) / (r * sqrt(3.)) );
		x = FANLib::Math::round( (pos.x - r * ( (y & 1)^(z & 1) ) ) / (r * 2) );
	}

	double HexNet::calcZDiff() {
		return r * sqrt(8./3.);
	}

}
