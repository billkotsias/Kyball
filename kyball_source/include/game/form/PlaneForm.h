////////////
// PlaneForm - a 'Form' descendant
////////////

#pragma once
#ifndef PlaneForm_H
#define PlaneForm_H

#include "Form.h"
#include "game\GameplayCollection.h"
#include <string>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class PlaneForm : public Form, public GameplayCollection { /// <!!!> INHERITANCE ORDER IS IMPORTANT <!!!>

	private:

		Ogre::SceneNode* plane[4]; /// collision object's nodes

	public:

		// constructor
		// => name = name to give to the created entity
		//	  material = name of material to use
		//	  x,y = width, height of entity
		PlaneForm(std::string name, std::string material, double x = 8, double y = 8);
		virtual ~PlaneForm();

	};
}

#endif