// ----------------------------------------------------
// CosmosCreator - build standard cosmos infrastructure
// ----------------------------------------------------

#pragma once
#ifndef CosmosCreator_H
#define CosmosCreator_H

#include "map\MapOptions.h"
#include <deque>
#include <string>
#include <OgreMaterial.h>

namespace Ogre {
	class BillboardSet;
	class SceneManager;
	class Entity;
	class Frustum;
}

namespace P3D {

	class GameplayBase;
	class GameplayCollection;
	class Cannon;
	class Camera;
	class TextureShadowBuffer;

	class HDRListener : public Ogre::CompositorInstance::Listener
	{
	protected:
		int mVpWidth, mVpHeight;
		int mBloomSize;
		// Array params - have to pack in groups of 4 since this is how Cg generates them
		// also prevents dependent texture read problems if ops don't require swizzle
		float mBloomTexWeights[15][4];
		float mBloomTexOffsetsHorz[15][4];
		float mBloomTexOffsetsVert[15][4];
	public:
		HDRListener() {};
		virtual ~HDRListener() {};
		void notifyViewportSize(int width, int height);
		void notifyCompositor(Ogre::CompositorInstance* instance);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};

	class BloomListener : public Ogre::CompositorInstance::Listener
	{
	private:
		Ogre::GpuProgramParametersSharedPtr bloomParams;
		double blurWeight;
		double originalImageWeight;

		int mVpWidth, mVpHeight;
		float mBloomTexWeights[15][4];
		float mBloomTexOffsetsHorz[15][4];
		float mBloomTexOffsetsVert[15][4];
	public:
		BloomListener() : blurWeight(0.5), originalImageWeight(0.5) {};
		virtual ~BloomListener() {};

		void setBloomWeight(const double& value);
		void setOriginalWeight(const double& value);

		BloomListener* notifyViewportSize(int width, int height);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};

	class SharpenListener : public Ogre::CompositorInstance::Listener
	{
	private:
		Ogre::GpuProgramParametersSharedPtr bloomParams; /// historical name
		double sharpen;
	public:
		SharpenListener() : sharpen(0.25) {};
		virtual ~SharpenListener() {};
		void setSharpen(const double& value);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};

	class CosmosCreator {

	private:

		/// compositor stuff
		HDRListener* hdrListener;
		BloomListener* bloomListener;
		SharpenListener* sharpenListener;

		MapOptions mapOptions;
		std::deque<Ogre::MaterialPtr> customMaterials;			/// } keep current Cosmos custom created materials,
		std::deque<Ogre::Frustum*> decalFrustums;				/// } frustums,
		std::deque<std::string> customTextures;					/// } textures,
		std::deque<TextureShadowBuffer*> customShadowBuffers;	/// } and shadow-buffers, to remove later altogether cause they are "inter-connected"

		// internal service members
		Ogre::SceneManager* sceneManager; /// hold this as it's frequently used
		std::string cosmosName(const std::string &);
		Ogre::Entity* buildCosmosEntity(std::string, Ogre::SceneNode*, int);
		GameplayCollection* cosmosCollection;

		void buildOnce();

	public:

		enum NetType {
			CUBE,
			HEX,
		};

		static std::vector<NetType> enumToNetType;

		CosmosCreator();
		~CosmosCreator();

		// custom shadows properties
		static const char* const SHADOW_POSTFIX;	/// postfix for material names accepting projected shadows
		static const unsigned int SHADOW_SIZE;		/// shadow texture dimensions(^2)
		static const char* const SHADOW_TEXTURE;	/// name of shadow texture
		static const char* const SHADOW_TEX_GROUP;	/// name of shadow texture group

		// maximum number of billboards for each set
		static const unsigned int BBSET_BALLS_NUM;

		// create 'Cosmos' scene
		// => MapOptions includes Cosmos ID info
		//	  number of views to create
		// <= builds Scene base through 'SceneBaseCreator'
		void create(MapOptions, int);

		// destroy last created 'Cosmos'
		// NOTE : all created 'GameplayBase's must have been destroyed before this call
		void uncreate();

		// build gameplay base for a player
		// NOTE		: 'create()' must first be issued
		// NOTE #2	: must be deleted by the user
		// => player number, starting at 0
		// <= pointer to created 'GameplayBase'
		GameplayBase* buildGameplayBase(unsigned int);
	};

}

#endif