// -----------
// Net - math definition of 'Gameplay's "net"
// -----------

#pragma once
#ifndef Net_H
#define Net_H

#include <math\Math.h>
#include <OgreVector3.h>
#include <utility>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class CollisionObject;

	// NetNode

	class NetNode {

	private:

		unsigned int _id; /// node coords compacted within an int; 32-bit int means 10 bits for each component = [-512,511)

	public:

		static const int intBits = sizeof(int) * 8;
		static const int shift = intBits / 3;

		NetNode();
		NetNode(int, int, int);
		inline unsigned int id() const { return _id; };

		inline bool operator<(const NetNode &other) const { return (_id < other.id()); };
		inline bool operator<=(const NetNode &other) const { return (_id <= other.id()); };
		inline bool operator==(const NetNode &other) const { return (_id == other.id()); };

		Ogre::Vector3 split() const; /// debug
	};


	// Net

	class Net {

	protected:

		double gridSize;
		double r;		/// half of gridSize, cached
		double zDiff;	/// cached z-difference between points <x,y,z> and <x,y,z+1> (should be x-y-z-independent)

	public:

		Net(double size) : gridSize(size), r(size/2) {};
		Net* init();	/// <MUST> be called right after construction
		virtual ~Net() {};


		// functions called in 'init()'
		virtual double calcZDiff() = 0; /// calculate zDiff value


		// find a "net" node and set as "unavailable"; node must satisfy the following requirements :
		// => pos = node must be as close as possible to this position
		//	  obj = distance of node from this 'CollisionObject' must be minimal
		// <= world coords of found node
		//	  'NetNode' id, to "release" when no longer needed to be reserved
		virtual std::pair<Ogre::Vector3, NetNode> findAndReserveNode(const Ogre::Vector3&, const CollisionObject* const);


		// find a suitable "net" node - used by 'findAndReserveNode()'
		virtual std::pair<Ogre::Vector3, NetNode> findNode(const Ogre::Vector3&, const CollisionObject* const) = 0;


		// set a node as "unavailable" (e.g already occupied)
		// => world coords
		// <= corresponding 'NetNode'
		virtual NetNode reserveNode(const Ogre::Vector3&) = 0;
		// => world coords
		virtual void reserveNode(const NetNode&) = 0;

		/// reverse effect of previous function
		virtual void releaseNode(const NetNode&) = 0;


		// convert NetNode (split components) to world coords
		// => NetNode x,y,z
		// <= world coords
		virtual Ogre::Vector3 netNodeToWorld(const int, const int, const int) = 0;

		// get nearest NetNode (split) to a world position
		// => world position
		// <= x,y,z components
		virtual void getNearestNode(const Ogre::Vector3&, int &, int &, int &) = 0;

		/// <TODO> : I could add functions to make <areas of nodes> unavailable e.g :
		/// - reserveNodes(Cube/Sphere)

		// reset nodes to "all available"
		virtual void reset() = 0;

		// is node unavailable?
		virtual bool isNodeReserved(const NetNode&) = 0;


		/// getters
		virtual Ogre::SceneNode* getOrigin() = 0; /// get origin for manipulation
		inline double getGridSize() { return gridSize; };
		inline double getHalfGridSize() { return r; };
		inline double getZDiff() { return zDiff; };
	};

}
#endif