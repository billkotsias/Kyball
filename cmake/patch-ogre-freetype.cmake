# Applied from the Ogre source root. Lets a nested Android build use the
# freetype target instead of a host FreeType.
file(READ CMake/Dependencies.cmake text)
if(NOT text MATCHES "KYBALL_FREETYPE")
	string(REPLACE "find_package(Freetype)"
"if(ANDROID AND TARGET freetype)
  set(FREETYPE_FOUND TRUE)
  set(FREETYPE_INCLUDE_DIRS \"\${freetype_SOURCE_DIR}/include\")
  set(FREETYPE_LIBRARIES freetype)
else()
  find_package(Freetype)
endif()
# KYBALL_FREETYPE"
		text "${text}")
	file(WRITE CMake/Dependencies.cmake "${text}")
endif()

# NDK r26+ (API 24) already defines pthread_barrier_t. Ogre 1.10's fallback
# struct and functions clash with it. Keep the fallback for older Android and Apple.
file(READ OgreMain/include/Threading/OgreBarrier.h barrier_h)
if(NOT barrier_h MATCHES "KYBALL_BARRIER")
	string(REPLACE
		"#if defined(ANDROID) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS"
		"#if (defined(ANDROID) && __ANDROID_API__ < 24) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS /* KYBALL_BARRIER */"
		barrier_h "${barrier_h}")
	file(WRITE OgreMain/include/Threading/OgreBarrier.h "${barrier_h}")
endif()
file(READ OgreMain/src/Threading/OgreBarrierPThreads.cpp barrier_cpp)
if(NOT barrier_cpp MATCHES "KYBALL_BARRIER")
	string(REPLACE
		"#if defined(ANDROID) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS"
		"#if (defined(ANDROID) && __ANDROID_API__ < 24) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS /* KYBALL_BARRIER */"
		barrier_cpp "${barrier_cpp}")
	file(WRITE OgreMain/src/Threading/OgreBarrierPThreads.cpp "${barrier_cpp}")
endif()

# DXT blocks are 4x4. Drivers reject compressed uploads when width or height
# is not a multiple of 4 (logo.DDS is 1233x345, Cloud.DDS is 151x151), so those
# menu images render black. Decompress them the same way Ogre already does when
# the render system has no DXT support.
file(READ OgreMain/src/OgreDDSCodec.cpp dds_cpp)
if(NOT dds_cpp MATCHES "KYBALL_DXT_ALIGN")
	string(REPLACE
		"                && !imgData->num_mipmaps))"
		"                && !imgData->num_mipmaps)
                || (header.width % 4) || (header.height % 4)) /* KYBALL_DXT_ALIGN */"
		dds_cpp "${dds_cpp}")
	file(WRITE OgreMain/src/OgreDDSCodec.cpp "${dds_cpp}")
endif()
