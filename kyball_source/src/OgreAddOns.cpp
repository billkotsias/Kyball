#include "StdAfx.h"
#include "OgreAddOns.h"

#include "game\CosmosCreator.h"

#include <OgreSceneNode.h>

namespace P3D {

	void OgreAddOns::reenableCompositors(Ogre::Viewport* const viewport)
	{
		Ogre::CompositorChain* chain = Ogre::CompositorManager::getSingletonPtr()->getCompositorChain(viewport);
		for (int i = chain->getNumCompositors() - 1; i >= 0; --i)
		{
			Ogre::CompositorInstance* compositor = chain->getCompositor(i);
			compositor->setEnabled(false);
			compositor->setEnabled(true);
		}
	}

	Ogre::Vector3 OgreAddOns::getDerivedPosition(Ogre::SceneNode *node) {
		node->_update(true, true);
		return node->_getDerivedPosition();
	}

	void OgreAddOns::setDerivedPosition(Ogre::SceneNode *node, const Ogre::Vector3 *pos) {
		node->setPosition(worldToLocal(node, pos));
	}

	Ogre::Vector3 OgreAddOns::worldToLocal(Ogre::SceneNode *node, const Ogre::Vector3* const pos ) {

		Ogre::SceneNode* parentNode = node->getParentSceneNode();
		if (!parentNode) parentNode = node; /// in case it's the root we are talking about, or if it's just out of the SceneGraph

		return worldToLocalParent(parentNode, pos);
	}

	Ogre::Vector3 OgreAddOns::worldToLocalParent(Ogre::SceneNode *parentNode, const Ogre::Vector3* const pos ) {
		return parentNode->_getFullTransform().inverse() * (*pos);
		//return parentNode->_getDerivedOrientation().Inverse() * (*pos - parentNode->_getDerivedPosition()); /// PLAIN WRONG
	}

	Ogre::Quaternion OgreAddOns::getOrientation(Ogre::Vector3 vecRef, Ogre::Vector3 vec) {
		Ogre::Vector3 rotAxis = vec.crossProduct(vecRef);
		Ogre::Real angle = vec.dotProduct(vecRef);
		return Ogre::Quaternion(Ogre::Radian(acos(angle)), rotAxis);
	}

	void OgreAddOns::setOrientationYXZ(Ogre::Node* node, const double& xRot, const double& yRot) {
		static Ogre::Matrix3 rotMatrix;
		static Ogre::Quaternion quat;

		rotMatrix.FromEulerAnglesYXZ(Ogre::Degree(xRot), Ogre::Degree(yRot), Ogre::Radian(0));
		quat.FromRotationMatrix(rotMatrix);
		node->setOrientation( quat );
	}

	void OgreAddOns::setOrientationYXZ(Ogre::Node* node, const double& xRot, const double& yRot, const double& zRot) {
		static Ogre::Matrix3 rotMatrix;
		static Ogre::Quaternion quat;

		rotMatrix.FromEulerAnglesYXZ(Ogre::Degree(xRot), Ogre::Degree(yRot), Ogre::Degree(zRot));
		quat.FromRotationMatrix(rotMatrix);
		node->setOrientation( quat );
	}

	void OgreAddOns::reloadAllTextures(std::string group) {
		reloadAllResources<Ogre::TextureManager, Ogre::TexturePtr>(group);
	}

	void OgreAddOns::reloadAllMeshes(std::string group) {
		reloadAllResources<Ogre::MeshManager, Ogre::MeshPtr>(group);
	}

	template<class Manager, class ResourcePtr>
	void OgreAddOns::reloadAllResources(std::string group) {
		Manager* man = Manager::getSingletonPtr();

		Ogre::ResourceManager::ResourceMapIterator resourceMapIterator = man->getResourceIterator();
		while (resourceMapIterator.hasMoreElements()) {
			ResourcePtr resPtr = resourceMapIterator.getNext();
			if (resPtr->isReloadable() && (resPtr->getGroup() == group))
			{
				resPtr->reload();
			}
		}
	}

	void OgreAddOns::removeParentScale(Ogre::Node *node, const Ogre::Vector3& localScale) {
		node->setScale( localScale / node->getParent()->getScale() );
	}
	void OgreAddOns::removeParentScale(Ogre::Node **nodes, unsigned int num, const Ogre::Vector3& localScale) {
		for (unsigned int i = 0; i < num; ++i) {
			removeParentScale(nodes[i], localScale);
		}
	}

	std::set<Ogre::MaterialPtr>& OgreAddOns::getMaterialsIn(std::set<Ogre::MaterialPtr>& populate, Ogre::SceneNode* node, bool iterateChildren)
	{
		/// get all Entities->SubEntities->Materials attached to current node
		Ogre::SceneNode::ObjectIterator objects = node->getAttachedObjectIterator();
		while (objects.hasMoreElements())
		{
			Ogre::MovableObject* movable = objects.getNext();
			if (movable->getTypeFlags() != Ogre::SceneManager::ENTITY_TYPE_MASK) continue;
			Ogre::Entity* entity = (Ogre::Entity*)movable;
			for (int i = entity->getNumSubEntities() - 1; i >= 0; --i) {
				populate.insert( entity->getSubEntity(i)->getMaterial() );
			}
		}

		/// repeat for all children
		if (iterateChildren) {
			Ogre::SceneNode::ChildNodeIterator nodeChildren = node->getChildIterator();
			while (nodeChildren.hasMoreElements())
			{
				OgreAddOns::getMaterialsIn(populate, (Ogre::SceneNode*)nodeChildren.getNext(), true);
			}
		}

		return populate;
	}

	Ogre::MeshPtr OgreAddOns::createSphereMesh(Ogre::SceneManager* sceneMgr, const std::string& strName, const float& r, const int& nRings, const int& nSegments, const bool& invertNormals)
	{
		Ogre::ManualObject* manual = sceneMgr->createManualObject(strName);
		manual->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	 
		float fDeltaRingAngle = (Ogre::Math::PI / nRings);
		float fDeltaSegAngle = (2 * Ogre::Math::PI / nSegments);
		unsigned short wVerticeIndex = 0 ;
	 
		// Generate the group of rings for the sphere
		for( int ring = 0; ring <= nRings; ring++ )
		{
			float r0 = r * sinf (ring * fDeltaRingAngle);
			float y0 = r * cosf (ring * fDeltaRingAngle);
	 
			// Generate the group of segments for the current ring
			for (int seg = 0; seg <= nSegments; seg++)
			{
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);
	 
				// Add one vertex to the strip which makes up the sphere
				manual->position( x0, y0, z0);
				Ogre::Vector3 normal = (invertNormals) ? Ogre::Vector3(-x0, -y0, -z0) : Ogre::Vector3(x0, y0, z0);
				normal.normalise();
				manual->normal(normal);
				manual->textureCoord((float) seg / (float) nSegments, (float) ring / (float) nRings);
	 
				if (ring != nRings)
				{
					 // each vertex (except the last) has six indicies pointing to it
					 manual->index(wVerticeIndex + nSegments + 1);
					 if (!invertNormals) manual->index(wVerticeIndex);
					 manual->index(wVerticeIndex + nSegments);
					 if (invertNormals) manual->index(wVerticeIndex);

					 manual->index(wVerticeIndex + nSegments + 1);
					 if (!invertNormals) manual->index(wVerticeIndex + 1);
					 manual->index(wVerticeIndex);
					 if (invertNormals) manual->index(wVerticeIndex + 1);
					 ++wVerticeIndex;
				}
			}
		}
		manual->end();

		/// -> to mesh
		Ogre::MeshPtr mesh = manual->convertToMesh(strName);
		sceneMgr->destroyManualObject( manual );
		mesh->_setBounds( Ogre::AxisAlignedBox( Ogre::Vector3(-r, -r, -r), Ogre::Vector3(r, r, r) ), false );
		mesh->_setBoundingSphereRadius(r);
		unsigned short src, dest;
		if (!mesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
			mesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);

		return mesh;
	}
}