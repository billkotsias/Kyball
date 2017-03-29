#include "StdAfx.h"
#include "game\form\CylinderForm.h"

#include "OgreBuilder.h"
#include "collision\Cylinder.h"

namespace P3D {

	CylinderForm::CylinderForm(std::string name, std::string material, double height, double diameter, bool visible, double fix) : Form(), GameplayCollection() {

		Ogre::SceneNode* p0 = newSceneNode(pivot, &Ogre::Vector3(0, -fix, 0));
		Ogre::SceneNode* p1 = newSceneNode(p0, &Ogre::Vector3::UNIT_Y);
		p0->setScale(diameter, height, diameter);

		/// build <collisionObject>
		collisionObject = new Cylinder(p0, p1, 0.5);

		/// build <MovableObject>
		if (visible) {
			storeMovable( OgreBuilder::createEntity( name, "cylinder.mesh", false, material, p0 ) );
		}
	}

	CylinderForm::~CylinderForm() {
	}

}