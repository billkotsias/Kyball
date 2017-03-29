#include "StdAfx.h"
#include "game\form\WallForm.h"

#include "OgreBuilder.h"
#include "collision\Polygon.h"

namespace P3D {

	WallForm::WallForm(std::string name, std::string material, double x, double y, double zFix, bool visible) : Form(), GameplayCollection() {

		/// build <collisionObject>
		pivot->setPosition(0,0,zFix);

		Ogre::SceneNode* p0 = newSceneNode(pivot, &Ogre::Vector3(0, y, 0));
		Ogre::SceneNode* p1 = newSceneNode(pivot, &Ogre::Vector3(-x, y, 0));
		Ogre::SceneNode* p2 = newSceneNode(pivot, &Ogre::Vector3(-x, 0, 0));
		Ogre::SceneNode* p3 = newSceneNode(pivot, &Ogre::Vector3(0, 0, 0));

		plane[0] = p0;
		plane[1] = p3;
		plane[2] = p2;
		plane[3] = p1;

		collisionObject = new Polygon(4, plane);

		/// build <MovableObject>
		if (visible) {
			Ogre::SceneNode* pBack = newSceneNode(pivot, &Ogre::Vector3(0, 0, 0));
			pBack->setScale(x,y,1);
			storeMovable( OgreBuilder::createEntity( name, "cube_back.mesh", false, material, pBack ) );
		}
	}

	WallForm::~WallForm() {
	}

}