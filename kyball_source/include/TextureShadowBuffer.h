//////////////////////
// TextureShadowBuffer - keep copy of a GPU texture buffer in CPU memory
//////////////////////

#pragma once
#ifndef TextureShadowBuffer_H
#define TextureShadowBuffer_H

#include <OgreHardwarePixelBuffer.h>
#include <string>

namespace FANLib {
	class Surface;
}

namespace P3D {

	class TextureShadowBuffer {

	protected:

		Ogre::HardwarePixelBufferSharedPtr textureBuffer;
		FANLib::Surface* surface;

	public:

		// constructor
		TextureShadowBuffer(Ogre::HardwarePixelBufferSharedPtr&);
		TextureShadowBuffer(const std::string&);
		void init(Ogre::HardwarePixelBufferSharedPtr&);
		~TextureShadowBuffer();

		inline FANLib::Surface* getSurface() { return surface; };

		// surface modifiers
		void clearSurface(unsigned __int32 = 0);

		// texture modifiers
		void updateTexture(); /// copy buffer to texture
	};
}

#endif