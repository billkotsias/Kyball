#include "StdAfx.h"
#include "TextureShadowBuffer.h"

#include <gfx\Surface.h>

#include <OgreTextureManager.h>
#include <OgreTexture.h>
#include <OgrePixelFormat.h>

namespace P3D {

	TextureShadowBuffer::TextureShadowBuffer(Ogre::HardwarePixelBufferSharedPtr &texBuffer) {
		init(texBuffer);
	}

	TextureShadowBuffer::TextureShadowBuffer(const std::string& texName) {
		Ogre::TextureManager* texMan = Ogre::TextureManager::getSingletonPtr();
		//texMan->load(texName, "GAMEPLAY", Ogre::TEX_TYPE_2D, 0, 1, false); /// resource must be pre-loaded by the caller!!!!!!!!!!
		init(((Ogre::TexturePtr)(texMan->getByName(texName)))->getBuffer(0, 0));
	}

	void TextureShadowBuffer::init(Ogre::HardwarePixelBufferSharedPtr &texBuffer) {

		textureBuffer = texBuffer;

		/// read texture's properties
		textureBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
		const Ogre::PixelBox &pb = textureBuffer->getCurrentLock();
		FANLib::Surface::PixelFormat format;
		switch (pb.format) {
			case Ogre::PF_A8R8G8B8:
				format = FANLib::Surface::ARGB_8888;
				break;
			case Ogre::PF_A8:
				format = FANLib::Surface::A_8;
				break;
			case Ogre::PF_L8:
			default:
				format = FANLib::Surface::GREYSCALE_8;
				break;
		}

		surface = new FANLib::Surface(format, pb.getWidth(), pb.getHeight());
		surface->copyFrom(pb.data);
		textureBuffer->unlock();
	}

	TextureShadowBuffer::~TextureShadowBuffer() {
		delete surface;
		surface = 0;
	}

	void TextureShadowBuffer::clearSurface(unsigned __int32 colour) {
		surface->fill(colour);
	}

	void TextureShadowBuffer::updateTexture() {

		textureBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		surface->copyTo(textureBuffer->getCurrentLock().data);
		textureBuffer->unlock();
	}

}