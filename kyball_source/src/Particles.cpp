#include "StdAfx.h"
#include "Particles.h"

#include "SceneBaseCreator.h"
#include <OgreParticleSystem.h>
#include "OgreLinearForceAffector.h" /// <!!!>
#include <OgreSceneManager.h>
#include <OgreBuilder.h>
#include <OgreStringInterface.h>
#include <core\Utils.h>

namespace P3D {

	const std::string Particles::LINEAR_FORCE = "LinearForce"; /// discovered in "OgreLinearForceAffector.cpp"

	void Particles::ScaleSystem(Ogre::ParticleSystem* system, double scale)
	{
		if (scale == 1.) return; /// frig off
		//bool debug = false;
		//if (system->getName() == "Base/Fireworks") {
		//	std::cout << system->getName() << "\n";
		//	std::cout << system->getNumEmitters() << "\n";
		//	debug = true;
		//}

		/// - scale size
		system->setDefaultWidth( system->getDefaultWidth() * scale);
		system->setDefaultHeight( system->getDefaultHeight() * scale);
		
		/// - scale velocity
		for (int j = system->getNumEmitters() - 1; j >= 0; --j)
		{
			Ogre::ParticleEmitter* emitter = system->getEmitter(j);
			emitter->setMinParticleVelocity( emitter->getMinParticleVelocity() * scale );
			emitter->setMaxParticleVelocity( emitter->getMaxParticleVelocity() * scale );
			emitter->setPosition( emitter->getPosition() * scale );
			//std::cout << "Emitter type:" + emitter->getType() + "\n";
			if (emitter->getType() == "Box") {
				Ogre::Real oldValue;
				FANLib::Utils::fromString(oldValue, emitter->getParameter("width"));
				emitter->setParameter("width", FANLib::Utils::toString( oldValue * scale ));
				FANLib::Utils::fromString(oldValue, emitter->getParameter("height"));
				emitter->setParameter("height", FANLib::Utils::toString( oldValue * scale ));
				FANLib::Utils::fromString(oldValue, emitter->getParameter("depth"));
				emitter->setParameter("depth", FANLib::Utils::toString( oldValue * scale ));
				//Ogre::ParameterList paramsList = emitter->getParameters();
				//Ogre::ParameterList::const_iterator it = paramsList.begin();
				//while (it != paramsList.end()) {
				//	std::cout << "Box param:" + it->name + " " + FANLib::Utils::toString(it->paramType) + "\n";
				//	++it;
				//}
			}
		}
		/// - scale 'LinearForce' affectors
		for (int j = system->getNumAffectors() - 1; j >= 0; --j) {
			Ogre::LinearForceAffector* linear = (Ogre::LinearForceAffector*)(system->getAffector(j));
			if (linear->getType() != LINEAR_FORCE) continue; /// with out this, a crash could occur
			linear->setForceVector( linear->getForceVector() * scale );
		}
	}

	//

	Particles::Particles(const std::string& systemName, Ogre::SceneNode* parent, Ogre::Vector3& position, double scale, double fastForward, bool shadows)
	{
		static unsigned systemNum = 0;

		/// create & scale
		system = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createParticleSystem(systemName + FANLib::Utils::toString(systemNum++), systemName);
		system->setCastShadows(shadows);
		static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingletonPtr()->getByName( system->getMaterialName() ))->setReceiveShadows(false);
		ScaleSystem(system, scale);

		/// add in scene
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();
		Ogre::SceneNode* systemNode = sceneMan->createSceneNode();
		systemNode->attachObject(system);
		systemNode->setPosition(position);
		parent->addChild(systemNode);

		/// fast forward?
		if (fastForward) system->fastForward(fastForward);
	}

	Particles::~Particles() {
		OgreBuilder::destroySceneNode( system->getParentSceneNode() );
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->destroyParticleSystem( system );
	}
}