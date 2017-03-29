#include "StdAfx.h"
#include "game\cannon\CannonSwarm.h"

#include "OgreBuilder.h"
#include "OgreAddOns.h"
#include "tween\TLinear.h"

namespace P3D {

	CannonSwarm::CannonSwarm(const std::string& name, Ogre::SceneNode* parent) {
		node = parent->createChildSceneNode();
		entity = OgreBuilder::createEntity(name, "cannon_swarm.mesh", false, "", node);
		node->setVisible(false);

		material = ((Ogre::MaterialPtr)(Ogre::MaterialManager::getSingleton().getByName("cannon_swarm")))->clone(name + "_material");
		entity->getSubEntity(0)->setMaterial(material);
		pass = material->getTechnique(0)->getPass(0);
		pass->setFog(true);

#ifdef _DEBUG
		fin = 0;
		got = 0;
#endif
	}

	CannonSwarm::~CannonSwarm() {

		deleteTweens(); /// must be called prior to any tweens objects' deletion

		/// destroy visual subelements
		OgreBuilder::destroyMovable(entity);
		OgreBuilder::destroySceneNode(node);

		/// destroy personal material
		Ogre::MaterialManager::getSingletonPtr()->remove(material->getName());
	}

	void CannonSwarm::fadeIn(const Ogre::Vector3& pos, const Ogre::ColourValue& endCol, double sScale, double eScale, double time) {

#ifdef _DEBUG
		if (got) {
			std::cout<<"CannonSwarm : got!\n";
		}
#endif
		node->setVisible(true);

		/// set starting position
		OgreAddOns::setDerivedPosition(node, &pos);

		/// set colours/scale
		pass->setSelfIllumination(Ogre::ColourValue::ZERO);
		node->setScale(Ogre::Vector3(sScale,sScale,sScale));

		/// - fade-in
#ifdef _DEBUG
		fin = registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			pass, endCol, time, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
#else
		registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			pass, endCol, time, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
#endif

		/// - scale-in
		registerTween(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			node, Ogre::Vector3(eScale,eScale,eScale), time, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
	}

	void CannonSwarm::gotoLaunch(const Ogre::Vector3& pos, double time1, double time2) {

#ifdef _DEBUG
		if (fin) {
			std::cout<<"CannonSwarm : fin!\n";
		}
#endif

		deleteTweens(); /// in case has the mouse button permanently pressed

		/// create tweens
		TPlaylist* list;

		/// list 1 : position & scale
		list = new TPlaylist();
		/// - 1. position
		Ogre::Vector3 endPos = OgreAddOns::worldToLocal(node, &pos);
		list->add(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			node, endPos, time1, &Ogre::SceneNode::getPosition, &Ogre::SceneNode::setPosition)
			);
		/// - 2. scale
		list->add(new TLinear<Ogre::SceneNode, Ogre::Vector3>(
			node, Ogre::Vector3(1.5,1.5,1.5), time2, &Ogre::SceneNode::getScale, &Ogre::SceneNode::setScale)
			);
		/// - register list
		registerList(list);
#ifdef _DEBUG
		got = list;
#endif

		/// tween 2 : delay & fade out
		registerTween(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			pass, Ogre::ColourValue::ZERO, time2, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination, time1)
			);
	}

#ifdef _DEBUG
	void CannonSwarm::playlistFinished(TPlaylist* list) {
		if (list == fin) fin = 0;
		if (list == got) got = 0;
		this->TPlayed::playlistFinished(list);
	}
#endif
}