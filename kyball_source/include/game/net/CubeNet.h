// ------
// CubeNet - cube net; waste of space but easier to grasp
// ------

#pragma once
#ifndef CubeNet_H
#define CubeNet_H

#include "BasicNet.h"

namespace P3D {

	class CubeNet : public BasicNet {

	public:

		CubeNet(double size, Ogre::SceneNode* node);

		/// overrides :
		virtual Ogre::Vector3 netNodeToWorld(const int, const int, const int);
		virtual void getNearestNode(const Ogre::Vector3&, int &, int &, int &);
		virtual double calcZDiff();

	};

}
#endif