////////////////
// ShadowManager - receives shadows from casters
////////////////

#pragma once
#ifndef ShadowManager_H
#define ShadowManager_H

#include "ShadowCaster.h"
#include <map>
#include <vector>
#include <string>

namespace Ogre {
	class Vector2;
}

namespace FANLib {
	class Surface;
	class Area;
}

namespace P3D {

	class TextureShadowBuffer;
	class DecalShadow;

	class ShadowManager {

	protected:

		/// "destination" texture
		TextureShadowBuffer* textureShadowBuffer; /// texture/shadow-buffer combo which is to receive the casters' shadows
		Ogre::Vector2* textureUnitSize;	/// caster <-> texture coords accordance
		Ogre::Vector2* offset;			/// position of (0,0) in texture

		/// "sources"
		std::map<ShadowCaster*, ShadowCaster::Type> casters;
		static std::map<ShadowCaster::Type, std::string> shadowTextures; /// shadow type <-> texture accordance
		std::vector<DecalShadow*> decalShadows;

	public:

		ShadowManager(TextureShadowBuffer*, Ogre::Vector2*, Ogre::Vector2*, double);
		~ShadowManager();

		void update();

		void registerCaster(ShadowCaster*, ShadowCaster::Type);
		void unregisterCaster(ShadowCaster*);
	};


	/// structure used by 'ShadowManager'
	class DecalShadow {

	protected:

		TextureShadowBuffer* textShadowBuffer; /// texture/shadow-buffer combo; the shadow "source"
		FANLib::Surface* surface;
		FANLib::Area* sourceArea;

	public:

		DecalShadow(const std::string&, const Ogre::Vector2*);
		~DecalShadow();

		inline FANLib::Area* getArea() { return sourceArea; };
		inline FANLib::Surface* getSurface() { return surface; };
	};
}

#endif