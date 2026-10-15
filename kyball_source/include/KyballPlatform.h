#pragma once
#ifndef KYBALL_PLATFORM_H
#define KYBALL_PLATFORM_H

#include <string>

namespace Ogre { class Root; }

namespace P3D {

	void makeDirectory(const std::string& path);
	void reportError(const char* message);
	Ogre::Root* createOgreRoot();

}

#endif
