// -----------------------------------------------------
// FSLToOgre - easy FSLData to Ogre instances conversion
// -----------------------------------------------------

#pragma once
#ifndef FSLToOgre_H
#define FSLToOgre_H

#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreQuaternion.h>

namespace FANLib {
	class FSLClass;
	class FSLArray;
}

namespace P3D {

	class FSLToOgre {

	public:

		// RGB => ColourValue
		static Ogre::ColourValue rgbToColourValue(FANLib::FSLClass*);

		// Vector2 => Vector2
		static Ogre::Vector2 vector2ToVector2(FANLib::FSLClass*);

		// Vector2 => Vector3
		static Ogre::Vector3 vector2ToVector3(FANLib::FSLClass*);

		// Vector3 => Vector3
		static Ogre::Vector3 vector3ToVector3(FANLib::FSLClass*);

		// Vector4 => Vector4
		static Ogre::Vector4 vector4ToVector4(FANLib::FSLClass*);

		// Vector4 => Vector3 (eg. for scaling)
		static Ogre::Vector3 vector4ToVector3(FANLib::FSLClass*);

		// Array of 3 Euler angles (degrees) or 4 quaternion parameters => quaternion
		static Ogre::Quaternion arrayToQuaternion(FANLib::FSLArray*);

	};

}

#endif