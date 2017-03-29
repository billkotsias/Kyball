// ------
// HexNet - hexagonal net; the most compact
// ------

#pragma once
#ifndef HexNet_H
#define HexNet_H

#include "BasicNet.h"

namespace P3D {

	class HexNet : public BasicNet {

	public:

		HexNet(double size, Ogre::SceneNode* node);

		/// overrides :
		virtual Ogre::Vector3 netNodeToWorld(const int, const int, const int);
		virtual void getNearestNode(const Ogre::Vector3&, int &, int &, int &);
		virtual double calcZDiff();

	};

}
#endif