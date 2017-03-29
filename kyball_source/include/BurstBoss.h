// ---------
// BurstBoss - manage Ogre's "burst-type" particle systems
// ---------

#pragma once
#ifndef BurstBoss_H
#define BurstBoss_H

#include <vector>
#include <string>

namespace Ogre {
	class SceneNode;
	class ParticleSystem;
	class DeflectorPlaneAffector;
}

namespace P3D {

	class ParticleSystem : private std::pair<Ogre::ParticleSystem*, bool> {

	public: /// declare here anything you allow non-BurstBoss code to touch

		Ogre::ParticleSystem* getOgreSystem() { return first; };

		template <class Receiver>
		void setCallBack(Receiver* receiver, void(Receiver::*function)(void*)) {
			first->setCallBack(receiver, function);
		};

	private:

		ParticleSystem(Ogre::ParticleSystem* system, bool enabled) : std::pair<Ogre::ParticleSystem*, bool>(system, enabled) {
		};

		friend class BurstBoss;
	};

	class BurstBoss {

	public:

		enum Type {
			SMALL,
			BIG,
			BOB,
			SPARKS,
			STUCK,
			// new ones
			CUBE_WIN,
			COSMOS_WIN,
			BASE_FIREWORKS,

			_last,
		};

		//typedef std::pair<Ogre::ParticleSystem*, bool> ParticleSystem;

	private:

		typedef std::pair<Ogre::ParticleSystem*, std::vector<Ogre::DeflectorPlaneAffector*> > Template;
		std::vector< Template > templates; /// particle system types
		std::vector<unsigned int> prefabs; /// prefab this number of systems for each template
		std::vector<bool> denyDeflectors; /// systems without deflectors

		std::vector< std::vector<ParticleSystem> > systems; /// created systems for each type

		ParticleSystem* getOrCreateSystem(unsigned type);

		// create another system of a particular type
		// => Type (passed as unsigned int)
		ParticleSystem& addSystem(unsigned type);

		// enable/disable a system
		void enableSystem(ParticleSystem&);
		void disableSystem(ParticleSystem&);

		// => Type / scripted system name for this Type / scale / deny deflectors?
		void createTemplate(BurstBoss::Type, const std::string&, double, bool _denyDeflectors = false);

		/// 5 - clear templates to original form and destroy all created particle systems
		void reset();

	public:

		/// affectors string id
		static const std::string DEFLECTOR_PLANE;

		// constructor
		BurstBoss(double);
		~BurstBoss();

		// usage cycle:

		/// 0 - create a new, non-hardcoded template!
		/// <= new type
		unsigned createTemplate(const std::string& scriptName, double scale = 1., bool _denyDeflectors = true);

		/// 1 - a new deflector added to each of the previously created templates
		void newDeflector(const Ogre::Vector3&, const Ogre::Vector3&, double);

		/// 2 - OPTIONAL : "initialize" by adding at once a number of particle systems for each particle "template"
		void prefab();

		/// 3 - create bursts
		ParticleSystem* newBurst(unsigned type, Ogre::SceneNode*, const Ogre::Vector3& pos = Ogre::Vector3::ZERO, const Ogre::Quaternion& rot = Ogre::Quaternion::IDENTITY);
		ParticleSystem* newBurst(unsigned type, Ogre::Entity*, const std::string& boneName, const Ogre::Vector3& pos = Ogre::Vector3::ZERO, const Ogre::Quaternion& rot = Ogre::Quaternion::IDENTITY);
		void disableSystem(ParticleSystem*);

		/// 4 - check for finished bursts; must be done fluently (like, every frame?)
		void checkInactive();
		
	};

}
#endif