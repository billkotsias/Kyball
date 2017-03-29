#include "StdAfx.h"
#include "ShadowManager.h"

#include "gfx\Blitter.h"
#include "gfx\Area.h"
#include "gfx\Surface.h"
#include "TextureShadowBuffer.h"

#include <OgreVector2.h>

namespace P3D {

	std::map<ShadowCaster::Type, std::string> ShadowManager::shadowTextures; /// TODO : = init();

	ShadowManager::ShadowManager(TextureShadowBuffer* buffer, Ogre::Vector2* frustumSize, Ogre::Vector2* frustumOffset, double scale) : textureShadowBuffer(buffer) {

		/// build shadow type <-> texture accordance the 1st time
		if (!shadowTextures.size()) {
			shadowTextures[ShadowCaster::BLOB] = "blob_shadow.tga";
		}

		/// read "destination"
		FANLib::Surface* destSurface = textureShadowBuffer->getSurface();
		Ogre::Vector2 destSize(destSurface->getSizeX(), destSurface->getSizeY());

		textureUnitSize = new Ogre::Vector2();
		*textureUnitSize = destSize * scale / (*frustumSize);

		offset = new Ogre::Vector2();
		*offset = (*textureUnitSize) * (*frustumOffset) + destSize / 2;

		decalShadows.resize(shadowTextures.size());
		for (std::map<ShadowCaster::Type, std::string>::iterator it = shadowTextures.begin(); it != shadowTextures.end(); ++it) {
			DecalShadow* decal = new DecalShadow(it->second, textureUnitSize);
			decalShadows[it->first] = decal;
		}
	}

	ShadowManager::~ShadowManager() {
		delete textureUnitSize;
		delete offset;

		for (int i = decalShadows.size() - 1; i >= 0; --i) {
			delete decalShadows[i];
		}
		decalShadows.clear();
	}

	void ShadowManager::update() {

		textureShadowBuffer->clearSurface(0); /// test: 0xbb000000

		for (std::map<ShadowCaster*, ShadowCaster::Type>::iterator it = casters.begin(); it != casters.end(); ++it) {

			Ogre::Vector2 position, scaling;
			Ogre::Vector2 point1, point2;

			// TODO (future) : ShadowCaster should be able to cast different types of shadows!
			bool moreShadows;
			do {
				moreShadows = it->first->updateShadow(&position, &scaling);
				switch (it->second) {
					case ShadowCaster::BLOB:
						position = position * (*textureUnitSize) + (*offset);
						Ogre::Vector2 halfSize = scaling * (*textureUnitSize) / 2.;
						point1 = position - halfSize;
						point2 = position + halfSize;
						break;
				}
				FANLib::Area destArea(point1.x, point1.y, point2.x, point2.y);
				DecalShadow* shadow = decalShadows[it->second];

				FANLib::Blitter::draw(*textureShadowBuffer->getSurface(), destArea, *shadow->getSurface(), *shadow->getArea(), FANLib::Blitter::ADD_ALPHA);
			} while (moreShadows);
		}

		textureShadowBuffer->updateTexture();
	}

	void ShadowManager::registerCaster(P3D::ShadowCaster *caster, P3D::ShadowCaster::Type type) {
		casters[caster] = type;
	}

	void ShadowManager::unregisterCaster(P3D::ShadowCaster *caster) {
		casters.erase(caster);
	}


	//

	DecalShadow::DecalShadow(const std::string& textureName, const Ogre::Vector2* textUnitSize) {
		textShadowBuffer = new TextureShadowBuffer(textureName);
		surface = textShadowBuffer->getSurface();
		sourceArea = new FANLib::Area(0, 0, surface->getSizeX(), surface->getSizeY());
	}

	DecalShadow::~DecalShadow() {
		delete textShadowBuffer;
		delete sourceArea;
	}

}