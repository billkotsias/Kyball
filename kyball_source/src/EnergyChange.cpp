#include "StdAfx.h"
#include "visual\EnergyChange.h"
#include "visual\VisualBoss.h"

#include "tween\TLinear.h"
#include "tween\TSin.h"
#include "tween\TCos.h"
#include "OgreBuilder.h"
#include "OgreAddOns.h"

#include "sound\Sound.h"
#include "visual\CallEvery.h"

namespace P3D {

	EnergyChange::EnergyChange(Ogre::SceneNode* parent, const Ogre::Vector3& pos, const Ogre::Vector3& scale, double ttl, double rnd, const Ogre::Quaternion& rot, bool playSound, double delay) : timeToDie(false), totalTime(ttl), timeToLive(ttl + delay)
	{
		static const double FADE_IN_TIME = 0.1; /// percentage of fade-out (ttl) time
		static const double FADE_IN_SCALING = 0.5; /// initial percentage of normal scaling
		static const double FADE_OUT_SCALING = 0.9; /// added percentage of normal scaling

		this->pos = pos;

		/// - create and orient node
		node = parent->createChildSceneNode();
		OgreAddOns::setDerivedPosition(node, &pos);
		node->setOrientation(rot);

		ttl *= 1 + rnd * rand() / (double)RAND_MAX;

		/// - create entity & custom material
		Ogre::Entity* ent = OgreBuilder::createEntity(node->getName(), "energy_change.mesh", false, "", node);
		myMat = ent->getSubEntity(0)->getMaterial()->clone(node->getName());
		ent->setMaterial(myMat);

		if (!myMat->getBestTechnique()) myMat->load();
		Ogre::Pass* pass = myMat->getBestTechnique()->getPass(0);

		/// <tweenings>
		TPlaylist* list;

		/// "alpha" tween
		list = new TPlaylist();
		pass->setSelfIllumination(Ogre::ColourValue::Black);
		list->add(new TSin<Ogre::Pass, Ogre::ColourValue>(
			pass, Ogre::ColourValue::White, ttl * FADE_IN_TIME, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination, delay)
			);
		list->add(new TLinear<Ogre::Pass, Ogre::ColourValue>(
			pass, Ogre::ColourValue::Black, ttl, &Ogre::Pass::getSelfIllumination, &Ogre::Pass::setSelfIllumination)
			);
		registerList(list);

		/// scale tween
		list = new TPlaylist();
		node->setScale(scale * FADE_IN_SCALING);
		list->add(new TSin<Ogre::Node, Ogre::Vector3>(
			node, scale, ttl * FADE_IN_TIME, &Ogre::Node::getScale, &Ogre::Node::setScale, delay)
			);
		list->add(new TLinear<Ogre::Node, Ogre::Vector3>(
			node, scale * (1 + FADE_OUT_SCALING), ttl, &Ogre::Node::getScale, &Ogre::Node::setScale)
			);
		registerList(list);

		VisualBoss::getSingletonPtr()->insert(this);
		if (playSound) Sound::CHANGE_ENERGY->play(pos);
		//if ( !Sound::isPlaying( Sound::CHANGE_ENERGY->lastPlayingVoice) ) Sound::CHANGE_ENERGY->play(pos);
		//std::cout << "Energy Sound handle=" << Sound::CHANGE_ENERGY->play(pos) << " " << pos << "\n";
		//new CallEvery<EnergyChange>(this, &EnergyChange::playShit, 0.1);
	}
	//void EnergyChange::playShit() {
	//	std::cout << "Energy Sound handle=" << Sound::CHANGE_ENERGY->play(pos) << " " << pos << "\n";
	//}

	EnergyChange::~EnergyChange() {
		deleteTweens();
		OgreBuilder::destroyMovable( node->getAttachedObject(0) );
		OgreBuilder::destroySceneNode( node );
		Ogre::MaterialManager::getSingletonPtr()->remove(myMat->getHandle());
	}

	bool EnergyChange::run(double time) {
		if (timeToDie) return true;

		timeToLive -= time;
		node->yaw( Ogre::Radian(sin(Ogre::Math::TWO_PI * 0.01 * (totalTime - timeToLive) / totalTime)) );
		return false;
	};

	void EnergyChange::playlistFinished(TPlaylist* list) {
		this->TPlayed::playlistFinished(list);
		if (lists.empty()) {
			timeToDie = true;
		}
	}

}