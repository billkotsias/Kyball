#include "StdAfx.h"
#include "BurstBoss.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"
#include <OgreSceneManager.h>
#include <core\Utils.h>
#include <OgreParticleSystem.h>
#include "OgreDeflectorPlaneAffector.h" /// <!!!>
#include "OgreLinearForceAffector.h" /// <!!!>
#include "Particles.h"
#include "OgreMovableObject.h"

namespace P3D {

	const std::string BurstBoss::DEFLECTOR_PLANE = "DeflectorPlane"; /// discovered in "OgreDeflectorPlaneAffector.cpp"

	BurstBoss::BurstBoss(double scale) {

		templates.resize(BurstBoss::_last);
		systems.resize(BurstBoss::_last);
		denyDeflectors.resize(BurstBoss::_last);
		prefabs.resize(BurstBoss::_last);

		/// it's stupid, but templates have to be recreated every time instead of just once
		/// that is... unless of course 'BurstBoss' was turned into a Singleton... as it SHOULD be!
		/// <NOTE SOS> : BURSTBOSS is a singleton because there are different deflectors PER BASE!<!>!
		/// so maybe, I should split BurstBoss into... BurstBoss and Deflectors Applier!!!
		createTemplate(BurstBoss::SMALL, "Burst/Small", scale);
		createTemplate(BurstBoss::BIG, "Burst/Big", scale);
		createTemplate(BurstBoss::BOB, "Burst/Bob", scale);
		createTemplate(BurstBoss::SPARKS, "Burst/Sparks", scale);
		createTemplate(BurstBoss::STUCK, "Burst/Stuck", scale);
		createTemplate(BurstBoss::CUBE_WIN, "Cube/Win", scale);
		createTemplate(BurstBoss::COSMOS_WIN, "Cosmos/Win/Corners", 1., true); /// emitted in local space
		createTemplate(BurstBoss::BASE_FIREWORKS, "Base/Fireworks", 1., true); /// MUST be emitted in local space, due to emit_emitter!

		/// number of prefab inactive explosions for every gameplay base
		prefabs[BurstBoss::SMALL] = 10;
		prefabs[BurstBoss::BIG] = 10;
		prefabs[BurstBoss::BOB] = 5;
		prefabs[BurstBoss::SPARKS] = 4;
		prefabs[BurstBoss::STUCK] = 1;
	}

	BurstBoss::~BurstBoss()
	{
		reset();

		/// destroy templates
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();
		for (int i = templates.size() - 1; i >= 0; --i) {
			sceneMan->destroyParticleSystem(templates.at(i).first);
		}
	}

	unsigned BurstBoss::createTemplate(const std::string& name, double scale, bool _denyDeflectors)
	{
		for (unsigned type = 0; type < templates.size(); ++type)
		{
			if (templates.at(type).first->getName() == name)  return type; /// already exists!
		}

		unsigned newType = templates.size();

		Ogre::ParticleSystem* system = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createParticleSystem(name, name);
		Template _template( system, std::vector<Ogre::DeflectorPlaneAffector*>() );
		templates.push_back( _template );
		Particles::ScaleSystem(system, scale);
		denyDeflectors.push_back( _denyDeflectors );

		systems.push_back( std::vector<ParticleSystem>() );
		addSystem( newType ); /// if requested, there will probably be at least one usage

		return newType;
	}

	void BurstBoss::createTemplate(BurstBoss::Type type, const std::string& name, double scale, bool _denyDeflectors) {
		Ogre::ParticleSystem* system = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createParticleSystem(name, name);
		//system->setCastShadows(false); // useless (YAOB)
		templates[type].first = system;
		Particles::ScaleSystem(system, scale);
		denyDeflectors[type] = _denyDeflectors;
	}

	void BurstBoss::newDeflector(const Ogre::Vector3& point, const Ogre::Vector3& normal, double bounce) {
		for (int i = templates.size() - 1; i >= 0; --i) {
			if (denyDeflectors[i]) continue;
			Template& pair = templates.at(i);
			Ogre::DeflectorPlaneAffector* def = (Ogre::DeflectorPlaneAffector*)pair.first->addAffector(DEFLECTOR_PLANE);
			pair.second.push_back(def); /// register affector to remove later
			def->setPlanePoint(point);
			def->setPlaneNormal(normal);
			def->setBounce(bounce);
		}
	}

	void BurstBoss::reset() {
		/// templates are not harmed!!! (except their added deflectors)

		/// clear templates
		for (int i = templates.size() - 1; i >= 0; --i) {
			Template& pair = templates.at(i);
			Ogre::ParticleSystem* system = pair.first;
			std::vector<Ogre::DeflectorPlaneAffector*>& affectors = pair.second;
			for (int i = affectors.size() - 1; i >= 0; --i) {
				Ogre::DeflectorPlaneAffector* affector = affectors.at(i);
				for (int j = system->getNumAffectors() - 1; j >= 0; --j) {
					if (system->getAffector(j) == affector) {
						system->removeAffector(j);
						break;
					}
				}
			}
			affectors.clear();
		}

		/// destroy particle systems
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();

		for (int i = systems.size() - 1; i >= 0; --i) {
			std::vector<ParticleSystem>& sysType = systems.at(i);
			for (int j = sysType.size() - 1; j >= 0; --j) {
				Ogre::ParticleSystem* system = sysType.at(j).first;
				Ogre::SceneNode* parent = system->getParentSceneNode();
				system->detatchFromParent();
				sceneMan->destroyParticleSystem(system);	/// destroy system
				OgreBuilder::destroySceneNode(parent);		/// destroy parent SceneNode
			}
		}
		systems.clear();
	}

	ParticleSystem& BurstBoss::addSystem(unsigned int type) {
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();

		Ogre::ParticleSystem* _template = templates.at(type).first;
		std::vector<ParticleSystem>& sysType = systems.at(type); /// created systems of this type

		Ogre::ParticleSystem* newSystem = sceneMan->createParticleSystem(
			_template->getName() + FANLib::Utils::toString(sysType.size()), (size_t)0);
		//newSystem->setCastShadows( _template->getCastShadows() ); // useless (YAOB)
		*newSystem = *_template; /// copy template!
		for (int i = newSystem->getNumEmitters() - 1; i >= 0; --i) { /// initially disabled
			newSystem->getEmitter(i)->setEnabled(false);
		}

		sysType.push_back( ParticleSystem(newSystem,false) );
		return sysType.back(); /// leave this as is, it's excellent!
	}

	void BurstBoss::checkInactive() {
		for (int j = systems.size() - 1; j >= 0; --j) {
			std::vector<ParticleSystem>& sysType = systems.at(j); /// created systems of this type

			for (int i = sysType.size() - 1; i >= 0; --i) {
				ParticleSystem& pair = sysType.at(i);
				if (!pair.second) continue; /// already inactive, skip this

				Ogre::ParticleSystem* system = pair.first;
				if (system->getNumParticles()) continue; /// some particles are still alive, skip this

				bool isActive = false;
				for (int j = system->getNumEmitters() - 1; j >= 0; --j) {
					if (system->getEmitter(j)->getEnabled()) {
						isActive = true; /// an emitter is active, don't change status
						break;
					}
				}
				if (!isActive) disableSystem(pair);
			}
		}
	}

	void BurstBoss::disableSystem(ParticleSystem* pair) {
		disableSystem( *pair );
	}

	void BurstBoss::disableSystem(ParticleSystem& pair)
	{
		Ogre::ParticleSystem* system = pair.first;
		for (int i = system->getNumEmitters() - 1; i >= 0; --i) {
			system->getEmitter(i)->setEnabled(false);
		}

		/// further stop system from updating by removing from scene
		Ogre::SceneNode* systemParent;
		if ( ( systemParent = system->getParentSceneNode() ) == static_cast<Ogre::SceneNode*>(system->getParentNode()) ) {
			Ogre::SceneNode* parent = systemParent->getParentSceneNode();
			if (parent) parent->removeChild( systemParent ); /// leave system's parent SceneNode alone
		} else {
			system->detatchFromParent();
		}

		/// mark as disabled
		pair.second = false;
	}

	void BurstBoss::enableSystem(ParticleSystem& pair) {
		/// reenable system emitters
		Ogre::ParticleSystem* system = pair.first;
		for (int i = system->getNumEmitters() - 1; i >= 0; --i) {
			system->getEmitter(i)->setEnabled(true);
		}
		pair.second = true;
	}

	ParticleSystem* BurstBoss::getOrCreateSystem(unsigned type)
	{
		std::vector<ParticleSystem>& sysType = systems.at(type); /// created systems of this type

		/// get an inactive one
		ParticleSystem* pair;
		for (int i = sysType.size() - 1; i >= 0; --i)
		{
			pair = &sysType.at(i);
			if (pair->second) continue; /// already active
			return pair;
		}

		/// none found, must create some more
		return &addSystem(type);
	}

	ParticleSystem* BurstBoss::newBurst(unsigned type, Ogre::Entity* entity, const std::string& boneName, const Ogre::Vector3& position, const Ogre::Quaternion& rotation)
	{
		ParticleSystem* pair = getOrCreateSystem(type);
		Ogre::ParticleSystem* inactive = pair->first;

		/// check if system is already attached (can only be a 'SceneNode') to DESTROY it!
		if ( inactive->isAttached() ) {
			OgreBuilder::destroySceneNode(inactive->getParentSceneNode()); /// system detached from parent
		}
		entity->attachObjectToBone(boneName, inactive, rotation, position); /// attached to TagPoint
		//inactive->setCastShadows(false);
		//entity->setCastShadows(false); // unfortunately, that's YAOB (yet another Ogre bug): system attached to bone clones entity's 'cast-shadows flag'

		/// finish
		enableSystem(*pair);
		return pair;
	}

	ParticleSystem* BurstBoss::newBurst(unsigned type, Ogre::SceneNode* parent, const Ogre::Vector3& position, const Ogre::Quaternion& rotation)
	{
		ParticleSystem* pair = getOrCreateSystem(type);
		Ogre::ParticleSystem* inactive = pair->first;

		/// check system's parent is a 'SceneNode'
		Ogre::SceneNode* systemParent;
		if ( inactive->isAttached() ) {
			systemParent = inactive->getParentSceneNode();
		} else {
			systemParent = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createSceneNode();
			systemParent->attachObject(inactive);
		}
		systemParent->setPosition(position);
		systemParent->setOrientation(rotation);
		parent->addChild(systemParent);

		/// finish
		enableSystem(*pair);
		return pair;
	}

	void BurstBoss::prefab() {
		for (int i = prefabs.size() - 1; i >= 0; --i) {
			for (int j = prefabs.at(i); j > 0; --j) {
				addSystem(i);
			}
		}
	}
}