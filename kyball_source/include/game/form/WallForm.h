///////////
// WallForm - a 'Form' descendant
///////////

#pragma once
#ifndef WallForm_H
#define WallForm_H

#include "Form.h"
#include "game\GameplayCollection.h"
#include <string>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class WallForm : public Form, public GameplayCollection { /// <!!!> INHERITANCE ORDER IS IMPORTANT <!!!>

	private:

		Ogre::SceneNode* plane[4]; /// collision object's nodes

	public:

		// constructor
		// => name = name to give to the created entity
		//	  material = name of material to use
		//	  x,y = width, height of entity
		//	  zFix = added to z
		//	  build movable object? (or make invisible)
		WallForm(std::string name, std::string material, double x = 8, double y = 8, double zFix = 0, bool visible = true);
		virtual ~WallForm();

	};
}

#endif