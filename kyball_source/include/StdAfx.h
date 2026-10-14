// Precompiled-header stand-in. Included by the game translation units.

#pragma once
#ifndef KYBALL_STDAFX_H
#define KYBALL_STDAFX_H

#if defined(_WIN32)
#include <windows.h>
#endif

/// Ogre
#include <Ogre.h>
#include <OgreUTFString.h>
#include <OgreOverlay.h>
#include <OgreOverlayManager.h>
#include <OgreBorderPanelOverlayElement.h>
#include "OIS.h"

/// GUI
#include "MyGUI.h"
#include "MyGUI_OgrePlatform.h"

/// STL
#include <deque>
#include <string>
#include <map>
#include <stack>
#include <utility>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <fstream>

/// Sound
#include "FreeSL.h"

#endif
