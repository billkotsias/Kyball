// -----------------------------------------
// OgreBuilder - common Ogre objects builder
// -----------------------------------------

#pragma once
#ifndef OgreBuilder_H
#define OgreBuilder_H

#include <string>
#include <vector>
#include <deque>

namespace Ogre {
	class Entity;
	class SceneNode;
	class MovableObject;
	class Pass;
}

namespace FANLib {
	class FSLArray;
	class FSLClass;
}

namespace P3D {

	class GameplayCollection;
	class Visual;

	class OgreBuilder {

	private:

		// internal service function
		static inline Ogre::Entity* _createEntity(const std::string&, const std::string&, const bool&, Ogre::SceneNode*, const bool&);

		// fog stuff
		static std::deque<Ogre::Pass*> foggedPasses; /// keep record of fogged passes to undo later

	public:

		// create a common Ogre Entity from an Ogre mesh resource and attach it to an Ogre Node
		// => entName	= name to give to the new Entity
		//	  meshName	= full name of the mesh resource file (including any extension like ".mesh")
		//	  tanVecs	= build tangent vectors for this mesh?
		//	  matName/s	= name/array of names of the material/s to apply to this entity (default : "" = don't change original mesh's material)
		//	  node		= attach created entity to this node (0 = don't attach)
		//	  shadow	= does this entity cast a shadow?
		// <= new Entity
		static Ogre::Entity* createEntity(const std::string& entName, const std::string& meshName, const bool& tanVecs, const std::string& matName = "", Ogre::SceneNode* parent = 0, const bool& shadow = false);
		static Ogre::Entity* createEntity(const std::string& entName, const std::string& meshName, const bool& tanVecs, const FANLib::FSLArray* matNames, Ogre::SceneNode* parent = 0, const bool& shadow = false);

		// destroy a MovableObject in a "generic" way
		static void destroyMovable(Ogre::MovableObject*);

		// destroy a SceneNode in a "generic" way
		static void destroySceneNode(Ogre::SceneNode*);


		// add an id number to distinguish a name
		static std::string baseName(std::string, int);


		// create sphere
		static Ogre::Entity* createSphereEntity(const std::string& entName, const int& nRings, const int& nSegments, const bool& invertNormals, const std::string& matName = "", Ogre::SceneNode* parent = 0, const bool& shadow = false);


		/// <-------------------------->

		// create a SceneNode from an 'FSLClass Node', or from an array of 'Nodes'
		static void createNodes(FANLib::FSLArray*, Ogre::SceneNode* parent, GameplayCollection*); /// calls 'createNode'
		static Ogre::SceneNode* createNode(FANLib::FSLClass*, Ogre::SceneNode* parent, GameplayCollection*); /// recursive function

		static void setCamera(FANLib::FSLClass*, Ogre::Camera*);	/// set camera params from script; <NOTE> : "build camera" must be a separate function,
																	/// because camera name can't be changed

		static void setSkybox(FANLib::FSLClass*, Ogre::Camera*);
		static void setSkyplane(FANLib::FSLClass*);
		static void setFog(FANLib::FSLClass*, const Ogre::ColourValue&);

		static void setFogToMaterials(const Ogre::ColourValue& backgroundColour, const Ogre::Real& expDensity);
		static void setFogToMaterial(Ogre::MaterialPtr& material, const Ogre::ColourValue& backgroundColour, const Ogre::Real& expDensity);
		static void unsetFog(); /// remove previously set fog

		static Visual* createAnimator(FANLib::FSLClass*, Ogre::SceneNode*);
	};

}

#endif