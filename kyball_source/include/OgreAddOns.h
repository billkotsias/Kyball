/////////////
// OgreAddOns - Ogre add-ons (implemented externally, not in-engine)
/////////////

#pragma once
#ifndef OgreAddOns_H
#define OgreAddOns_H

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace Ogre {
	class Node;
	class SceneNode;
}

namespace P3D {

	class OgreAddOns {

	public:

		// re-enable all Compositors in a Viewport, useful when viewport's camera changes
		// NOTE : IT'S FRIGGIN' SLOW!!! DON'T USE!!!
		static void reenableCompositors(Ogre::Viewport* const);

		// convert world coords to a SceneNode's local coords
		// => node = parent of SceneNode to get world coords
		//	  pos = world coords
		// <= local coords
		static Ogre::Vector3 worldToLocalParent(Ogre::SceneNode*, const Ogre::Vector3* const);


		// convert world coords to a SceneNode's local coords
		// => node = SceneNode to get world coords
		//	  pos = world coords
		// <= local coords
		static Ogre::Vector3 worldToLocal(Ogre::SceneNode*, const Ogre::Vector3* const);


		// set the derived position of a scene node according to needed world-coords
		// => node = the scenenode in question
		//	  pos = wanted position of node in world-coords
		// <= the node's local position is altered in order to get wanted world-coords
		static void setDerivedPosition(Ogre::SceneNode*, const Ogre::Vector3* const);


		// get the derived position of a scene node, making sure it's updated regardless Ogre-"unawareness"
		// => node = the scenenode in question
		static Ogre::Vector3 getDerivedPosition(Ogre::SceneNode*);


		// get the orientation of a vector relatively to another
		static Ogre::Quaternion getOrientation(Ogre::Vector3, Ogre::Vector3);


		// set orientation to a node by YXZ degrees
		static void setOrientationYXZ(Ogre::Node*, const double&, const double&); /// optimized for z = 0
		static void setOrientationYXZ(Ogre::Node*, const double&, const double&, const double&);


		// reload all textures of a group
		static void reloadAllTextures(std::string group);
		static void reloadAllMeshes(std::string group);


		// remove parent's scale from a node (must have a parent!) + set local scale
		static void removeParentScale(Ogre::Node* node, const Ogre::Vector3& = Ogre::Vector3::UNIT_SCALE);
		static void removeParentScale(Ogre::Node** nodes, unsigned int, const Ogre::Vector3& = Ogre::Vector3::UNIT_SCALE); /// array of nodes of given size


		// get all materials found in (sub)objects attached to this node
		// => vector to populate with results (passed by reference)
		//	  SceneNode to search
		//	  iterate children
		// <= vector returned for possible function-chaining
		static std::set<Ogre::MaterialPtr>& getMaterialsIn(std::set<Ogre::MaterialPtr>& populate, Ogre::SceneNode* node, bool iterateChildren = true);


		// create manual sphere mesh
		static Ogre::MeshPtr createSphereMesh(Ogre::SceneManager* sceneMgr, const std::string& strName, const float& r, const int& nRings = 16, const int& nSegments = 16, const bool& invertNormals = false);


		//
		// create a shadow buffer of a texture pixel buffer in CPU memory
		///...?

	private:

		template<class Manager, class ResourcePtr>
		static void reloadAllResources(std::string group);
	};

	//

	class NodeWrapper {

	protected:

		Ogre::SceneNode* node;
		Ogre::Vector3 euler;

	public:

		NodeWrapper() : node(0) {
		}

		NodeWrapper(Ogre::SceneNode* n) {
			setNode(n);
		}

		void setNode(Ogre::SceneNode* n) {
			node = n;
			static Ogre::Matrix3 rotMatrix;
			static Ogre::Radian x,y,z;
			node->getOrientation().ToRotationMatrix(rotMatrix);
			rotMatrix.ToEulerAnglesYXZ(x,y,z);
			euler.x = Ogre::Degree(x).valueDegrees();
			euler.y = Ogre::Degree(y).valueDegrees();
			euler.z = Ogre::Degree(z).valueDegrees();
		};

		Ogre::SceneNode* getNode() { return node; };

		const Ogre::Vector3& getOrientation() const { return euler; };
		void setOrientation(const Ogre::Vector3& ori) {
			euler = ori;
			OgreAddOns::setOrientationYXZ(node, euler.x, euler.y, euler.z);
		};
	};
}

#endif