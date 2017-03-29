///////////////
// ShadowCaster - casts shadows for ShadowManager
///////////////

#pragma once
#ifndef ShadowCaster_H
#define ShadowCaster_H

namespace Ogre {
	class Vector2;
}

namespace P3D {

	class ShadowManager;

	class ShadowCaster {

	protected:

		ShadowManager* shadowManager;

	public:

		enum Type {
			BLOB,
		};

		ShadowCaster(ShadowManager*, ShadowCaster::Type);
		virtual ~ShadowCaster();

		// => x, y, scaleX, scaleY, passed by reference; must be updated by caster for manager
		// <= if you return 'true', this call will be re-issued for you to pass yet another shadow; return 'false' to exit
		virtual bool updateShadow(Ogre::Vector2*, Ogre::Vector2*) = 0;
	};
}

#endif