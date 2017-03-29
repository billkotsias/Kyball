// --------
// BasicNet - includes parameters required by all "normal" nets
// --------

#pragma once
#ifndef BasicNet_H
#define BasicNet_H

#include "Net.h"
#include <map>
#include <OgreVector3.h>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class BasicNet : public Net {

	protected:

#ifdef _DEBUG
		std::map<NetNode, Ogre::Vector3> reserved;	/// reserved nodes (DEBUG)
#else
		std::map<NetNode, bool> reserved;	/// reserved nodes
#endif

		Ogre::SceneNode* origin;			/// Net origin
		Ogre::SceneNode* pivot;				/// 'origin' child; used for Net to world convertions

		void pushNode(std::vector<std::pair<Ogre::Vector3, NetNode> >&, int, int, int); /// internal service function

	public:

		virtual ~BasicNet();

		// constructor
		// => minimum node distance
		//	  net origin
		BasicNet(double, Ogre::SceneNode*);

		virtual std::pair<Ogre::Vector3, NetNode> findNode(const Ogre::Vector3&, const CollisionObject* const);

		/// - node availability
		virtual NetNode reserveNode(const Ogre::Vector3&);
		virtual void reserveNode(const NetNode&);
		virtual void releaseNode(const NetNode&);
		virtual bool isNodeReserved(const NetNode&);
		virtual void reset();

		/// - getters
		virtual Ogre::SceneNode* getOrigin() { return origin; };
	};

}
#endif