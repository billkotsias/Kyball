#pragma once
#ifndef KYBALL_PLATFORM_H
#define KYBALL_PLATFORM_H

#include <string>

namespace Ogre { class Root; }

namespace P3D {

	void makeDirectory(const std::string& path);
	void reportError(const char* message);
	// resources.cfg paths are Windows-style. Prefix DATA_PATH and use '/' off Windows.
	std::string resourceLocation(const std::string& relative);
	Ogre::Root* createOgreRoot();

}

#endif
