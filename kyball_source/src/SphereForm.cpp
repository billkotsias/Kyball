#include "StdAfx.h"
#include "game\form\SphereForm.h"

#include "OgreBuilder.h"
#include "collision\Sphere.h"
#include "game\GameplayBase.h"

namespace P3D {

	SphereForm::SphereForm(GameplayBase* base, std::string name, std::string material, double d, bool visible) : Form(), GameplayCollection(),
		ShadowCaster(visible ? base->getShadowManager() : 0 /* MAGIA */, ShadowCaster::BLOB) {

		pivot->setScale(Ogre::Vector3(d));

		/// build <collisionObject>
		collisionObject = new Sphere(pivot, .5);

		/// build <MovableObject>
		if (visible) {
			storeMovable( OgreBuilder::createEntity( name, "cannon_swarm.mesh", false, material, pivot ) );
		}
	}

	SphereForm::~SphereForm() {
	}

	bool SphereForm::updateShadow(Ogre::Vector2* position, Ogre::Vector2* scaling) {
		Ogre::SceneNode* parent = pivot->getParentSceneNode();
		if (parent) {
			const Ogre::Vector3& pos = parent->getPosition();
			position->x = pos.x;
			position->y = pos.z;

			const Ogre::Vector3& sca = pivot->getScale();
			scaling->x = sca.x;
			scaling->y = sca.z;
		} else {
			*position = Ogre::Vector2::ZERO;
			*scaling = Ogre::Vector2::ZERO;
		}
		return false;
	}

}