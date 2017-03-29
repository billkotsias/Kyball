///////////////
// CylinderForm - a 'Form' descendant
///////////////

#pragma once
#ifndef CylinderForm_H
#define CylinderForm_H

#include "Form.h"
#include "BallState.h"
#include "ShadowCaster.h"
#include "game\GameplayCollection.h"

#include <string>

namespace Ogre {
	class SceneNode;
}

namespace P3D {

	class CylinderForm : public Form, public GameplayCollection { /// <!!!> INHERITANCE ORDER IS IMPORTANT <!!!>

	public:

		// constructor
		// => name = name to give to the created entity
		//	  material = name of material to use
		//	  h = height
		//	  d = diameter
		//	  visible
		//	  fix-aki
		//CylinderForm(std::string name, std::string material, double height = 1., double diameter = BallState::BALL_RADIUS * 2, double fix = BallState::BALL_RADIUS);
		CylinderForm(std::string name, std::string material, double height, double diameter, bool visible, double fix = BallState::BALL_RADIUS);
		virtual ~CylinderForm();
	};
}

#endif