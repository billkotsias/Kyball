#include "StdAfx.h"
#include "FSLToOgre.h"

#include <core\FastStr.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLArray.h>

#include <OgreMatrix3.h>

namespace P3D {

	Ogre::ColourValue FSLToOgre::rgbToColourValue(FANLib::FSLClass* fslClass) {
		return Ogre::ColourValue(fslClass->getReal("r"), fslClass->getReal("g"), fslClass->getReal("b"));
	}

	Ogre::Vector2 FSLToOgre::vector2ToVector2(FANLib::FSLClass *fslClass) {
		return Ogre::Vector2(fslClass->getReal("x"), fslClass->getReal("y"));
	}

	Ogre::Vector3 FSLToOgre::vector2ToVector3(FANLib::FSLClass *fslClass) {
		return Ogre::Vector3(fslClass->getReal("x"), fslClass->getReal("y"), 0);
	}

	Ogre::Vector3 FSLToOgre::vector3ToVector3(FANLib::FSLClass *fslClass) {
		return Ogre::Vector3(fslClass->getReal("x"), fslClass->getReal("y"), fslClass->getReal("z"));
	}

	Ogre::Vector4 FSLToOgre::vector4ToVector4(FANLib::FSLClass *fslClass) {
		return Ogre::Vector4(fslClass->getReal("x"), fslClass->getReal("y"), fslClass->getReal("z"), fslClass->getReal("w"));
	}

	Ogre::Vector3 FSLToOgre::vector4ToVector3(FANLib::FSLClass *fslClass) {
		FANLib::FSL::real w = fslClass->getReal("w");
		return Ogre::Vector3(fslClass->getReal("x") * w, fslClass->getReal("y") * w, fslClass->getReal("z") * w);
	}

	Ogre::Quaternion FSLToOgre::arrayToQuaternion(FANLib::FSLArray* fslArray)
	{
		switch (fslArray->getSize()) {
			case 4:
				return Ogre::Quaternion(fslArray->getReal(3), fslArray->getReal(0), fslArray->getReal(1), fslArray->getReal(2));
				break;
			case 3: {
					Ogre::Matrix3 matrix;
					matrix.FromEulerAnglesZXY(Ogre::Radian(Ogre::Degree(fslArray->getReal(2))),
											  Ogre::Radian(Ogre::Degree(fslArray->getReal(0))),
											  Ogre::Radian(Ogre::Degree(fslArray->getReal(1))));
					return Ogre::Quaternion(matrix);
					}
				break;
			default:
				return Ogre::Quaternion(); /// maybe we should get a runtime error here, or maybe a Log entry
				break;
		}
	}
}