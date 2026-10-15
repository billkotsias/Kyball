cmake_minimum_required(VERSION 3.16)

project(kyball LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug RelWithDebInfo Release" FORCE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GCC does not define _DEBUG. Ogre's IdString layout follows OGRE_DEBUG_MODE,
# which the headers recompute from _DEBUG, so the whole tree must agree.
# libc++ (Android NDK) cannot be built with -D_DEBUG; it breaks std::pair.
if(NOT ANDROID AND NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	add_compile_definitions(_DEBUG)
endif()

include(FetchContent)

# --- SDL2 (input, window, Android activity) ---
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL2
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG        release-2.30.10
	GIT_SHALLOW    TRUE
)

# --- OpenAL Soft ---
if(ANDROID)
	set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
else()
	set(LIBTYPE "SHARED" CACHE STRING "" FORCE)
endif()
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
FetchContent_Declare(openal
	GIT_REPOSITORY https://github.com/kcat/openal-soft.git
	GIT_TAG        1.23.1
	GIT_SHALLOW    TRUE
)

# --- Ogre 1.10 (fixed-function GL on Linux, GLES2 on Android) ---
set(OGRE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_BITES OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_PYTHON OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_JAVA OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_CSHARP OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_HLMS OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_TERRAIN OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_VOLUME OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_PAGING OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_PROPERTY OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_MESHLODGENERATOR OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_PLUGIN_BSP OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_PLUGIN_CG OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_PLUGIN_PCZ OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_PLUGIN_OCTREE OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_PLUGIN_PFX ON CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_OVERLAY ON CACHE BOOL "" FORCE)
set(OGRE_BUILD_RENDERSYSTEM_D3D9 OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_RENDERSYSTEM_D3D11 OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_RENDERSYSTEM_GL3PLUS OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_RENDERSYSTEM_GLES OFF CACHE BOOL "" FORCE)
set(OGRE_INSTALL_SAMPLES OFF CACHE BOOL "" FORCE)
set(OGRE_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(OGRE_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(OGRE_CONFIG_THREADS "0" CACHE STRING "" FORCE)
set(OGRE_CONFIG_ENABLE_FREEIMAGE ON CACHE BOOL "" FORCE)
set(OGRE_BUILD_DEPENDENCIES OFF CACHE BOOL "" FORCE)
if(ANDROID)
	set(OGRE_STATIC ON CACHE BOOL "" FORCE)
	set(OGRE_BUILD_RENDERSYSTEM_GL OFF CACHE BOOL "" FORCE)
	set(OGRE_BUILD_RENDERSYSTEM_GLES2 ON CACHE BOOL "" FORCE)
	set(OGRE_CONFIG_ENABLE_FREEIMAGE OFF CACHE BOOL "" FORCE)
else()
	set(OGRE_STATIC OFF CACHE BOOL "" FORCE)
	set(OGRE_BUILD_RENDERSYSTEM_GL ON CACHE BOOL "" FORCE)
	set(OGRE_BUILD_RENDERSYSTEM_GLES2 OFF CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(ogre
	GIT_REPOSITORY https://github.com/OGRECave/ogre.git
	GIT_TAG        v1.10.12
	GIT_SHALLOW    TRUE
	PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch-ogre-freetype.cmake
)

if(ANDROID)
	set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
	set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
	set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
	set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
	set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
	FetchContent_Declare(freetype
		GIT_REPOSITORY https://github.com/freetype/freetype.git
		GIT_TAG        VER-2-13-2
		GIT_SHALLOW    TRUE
	)
endif()

# MyGUI 3.2.2 matches the game's layout API. Its own CMake finder is not used;
# the engine and Ogre platform are compiled from this checkout below.
FetchContent_Declare(mygui
	GIT_REPOSITORY https://github.com/MyGUI/mygui.git
	GIT_TAG        MyGUI3.2.2
	GIT_SHALLOW    TRUE
)

if(ANDROID)
	FetchContent_MakeAvailable(freetype)
endif()
FetchContent_MakeAvailable(SDL2 openal ogre)
if(TARGET OgreMain)
	# Ogre writes export headers to the top-level binary dir when nested.
	target_include_directories(OgreMain PUBLIC "${CMAKE_BINARY_DIR}/include")
endif()
FetchContent_GetProperties(mygui)
if(NOT mygui_POPULATED)
	FetchContent_Populate(mygui)
endif()
execute_process(
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch-mygui-skins.cmake
	WORKING_DIRECTORY ${mygui_SOURCE_DIR}
)

if(NOT ANDROID)
	find_package(Freetype REQUIRED)
endif()
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

file(GLOB MYGUI_ENGINE_SRC CONFIGURE_DEPENDS "${mygui_SOURCE_DIR}/MyGUIEngine/src/*.cpp")
file(GLOB MYGUI_OGRE_SRC CONFIGURE_DEPENDS "${mygui_SOURCE_DIR}/Platforms/Ogre/OgrePlatform/src/*.cpp")

add_library(MyGUIEngine STATIC ${MYGUI_ENGINE_SRC})
target_include_directories(MyGUIEngine PUBLIC
	"${mygui_SOURCE_DIR}/MyGUIEngine/include"
	${FREETYPE_INCLUDE_DIRS}
)
target_compile_definitions(MyGUIEngine PUBLIC MYGUI_USE_FREETYPE MYGUI_DONT_REPLACE_NULLPTR)
if(TARGET Freetype::Freetype)
	target_link_libraries(MyGUIEngine PUBLIC Freetype::Freetype ${CMAKE_DL_LIBS})
else()
	target_link_libraries(MyGUIEngine PUBLIC freetype ${CMAKE_DL_LIBS})
endif()
target_compile_options(MyGUIEngine PRIVATE -w)

add_library(MyGUIOgrePlatform STATIC ${MYGUI_OGRE_SRC})
target_include_directories(MyGUIOgrePlatform PUBLIC
	"${mygui_SOURCE_DIR}/Platforms/Ogre/OgrePlatform/include"
	"${mygui_SOURCE_DIR}/MyGUIEngine/include"
	"${ogre_SOURCE_DIR}/OgreMain/include"
	"${ogre_BINARY_DIR}/include"
	"${CMAKE_BINARY_DIR}/include"
)
add_dependencies(MyGUIOgrePlatform OgreMain)
target_link_libraries(MyGUIOgrePlatform PUBLIC MyGUIEngine OgreMain)
target_compile_options(MyGUIOgrePlatform PRIVATE -w)

file(GLOB FANLIB_SRC CONFIGURE_DEPENDS fanlib_source/source/*.cpp)
add_library(fanlib STATIC ${FANLIB_SRC})
target_include_directories(fanlib PUBLIC fanlib_source/headers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(fanlib PRIVATE -fpermissive -Wno-narrowing -fno-strict-aliasing)
endif()

file(GLOB KYBALL_SRC CONFIGURE_DEPENDS kyball_source/src/*.cpp)
list(APPEND KYBALL_SRC
	kyball_source/include/OgreGpuCommandBufferFlush.cpp
	kyball_source/third_party/stb_vorbis.c
)
set_source_files_properties(kyball_source/third_party/stb_vorbis.c PROPERTIES LANGUAGE C COMPILE_OPTIONS "-w")

if(ANDROID)
	add_library(kyball SHARED ${KYBALL_SRC})
else()
	add_executable(kyball ${KYBALL_SRC})
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(kyball PRIVATE -fpermissive -Wno-narrowing -fno-strict-aliasing)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# Game code takes the address of short-lived Vector3 temporaries and uses
	# them only for the duration of the call (the same extension MSVC and g++ accept).
	# -w also hides MyGUI 3.2 singleton template warnings that otherwise flood the NDK log.
	target_compile_options(kyball PRIVATE -Wno-address-of-temporary -w)
endif()

target_include_directories(kyball PRIVATE
	kyball_source/include
	${ogre_SOURCE_DIR}/OgreMain/include
	${ogre_BINARY_DIR}/include
	${CMAKE_BINARY_DIR}/include
	${ogre_SOURCE_DIR}/Components/Overlay/include
	${ogre_SOURCE_DIR}/PlugIns/ParticleFX/include
	${mygui_SOURCE_DIR}/MyGUIEngine/include
	${mygui_SOURCE_DIR}/Platforms/Ogre/OgrePlatform/include
)
if(ANDROID)
	target_include_directories(kyball PRIVATE
		${ogre_SOURCE_DIR}/RenderSystems/GLES2/include
		${ogre_SOURCE_DIR}/RenderSystems/GLSupport/include
	)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	target_compile_definitions(kyball PRIVATE KYBALL_WINDOWED)
endif()
if(NOT ANDROID)
	target_compile_definitions(kyball PRIVATE "KYBALL_PLUGIN_DIR=\"$<TARGET_FILE_DIR:RenderSystem_GL>\"")
else()
	target_compile_definitions(kyball PRIVATE OGRE_STATIC_LIB KYBALL_STATIC_PLUGINS)
endif()

target_link_libraries(kyball PRIVATE
	fanlib
	MyGUIOgrePlatform
	MyGUIEngine
	OgreMain
	OgreOverlay
	Plugin_ParticleFX
	SDL2::SDL2
	SDL2::SDL2main
	OpenAL::OpenAL
	ZLIB::ZLIB
	Threads::Threads
	${CMAKE_DL_LIBS}
)
if(ANDROID)
	# OgreMain calls android_getCpuFamily but only adds the NDK header, not the source.
	add_library(cpufeatures STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
	target_include_directories(cpufeatures PUBLIC "${ANDROID_NDK}/sources/android/cpufeatures")
	# Static GLES2/GLSupport reference EGL and ANativeWindow. The NDK does not
	# propagate those into the static archives, so the shared library must name them.
	target_link_libraries(kyball PRIVATE RenderSystem_GLES2 cpufeatures EGL GLESv2 android log)
endif()

if(UNIX AND NOT ANDROID)
	# $ORIGIN is relative to the executable. A container build otherwise bakes
	# the mount path (/src/build/...) and the binary cannot find its libraries
	# on the host. --disable-new-dtags keeps DT_RPATH so FreeImage and zziplib
	# next to the executable are visible to OgreMain as well.
	target_link_options(kyball PRIVATE "-Wl,--disable-new-dtags")
	set_target_properties(kyball PROPERTIES
		BUILD_RPATH "\$ORIGIN/_deps/ogre-build/lib;\$ORIGIN/_deps/sdl2-build;\$ORIGIN/_deps/openal-build;\$ORIGIN/syslibs"
		INSTALL_RPATH "\$ORIGIN"
	)
endif()
