/////////////
// SphereForm - a 'Form' descendant
/////////////

#pragma once
#ifndef SphereForm_H
#define SphereForm_H

#include "Form.h"
#include "BallState.h"
#include "ShadowCaster.h"
#include "game\GameplayCollection.h"

#include <string>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class GameplayBase;

	class SphereForm : public Form, public GameplayCollection, public ShadowCaster { /// <!!!> INHERITANCE ORDER IS IMPORTANT <!!!>

	public:

		// constructor
		// => a 'GameplayBase', needed to get its 'ShadowManager'
		//	  name = name to give to the created entity
		//	  material = name of material to use
		//	  d = diameter
		SphereForm(GameplayBase* base, std::string name, std::string material, double d, bool visible);
		virtual ~SphereForm();

		// overrides
		virtual bool updateShadow(Ogre::Vector2*, Ogre::Vector2*);
	};
}

#endif