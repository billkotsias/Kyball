// -----------------------------
// P3DScripts - scripts launcher
// -----------------------------
// - loads and parses all game scripts
// - script data is accessible through public static variables

#pragma once
#ifndef P3DScripts_H
#define P3DScripts_H

#include <vector>

namespace FANLib {
	class FSLInstance;
}

namespace P3D {

	class P3DScripts {

	private:

		// static "constructor" function
		static FANLib::FSLInstance* parseFile(const char*);

		// "constructor"/"destructor"
		static void load();
		static void unload();

		friend class P3DApp;

		static const FANLib::FSLInstance* p3dExtro;

	public:

		static std::vector<Ogre::Light::LightTypes> lightTypes;

		// application scripts
		static const FANLib::FSLInstance* p3dLevels;	/// P3DLevels.fsl
		//static const FANLib::FSLInstance* p3dChapters;	/// P3DChapters.fsl
		static const FANLib::FSLInstance* p3dVars;		/// P3DVars.fsl
		static const FANLib::FSLInstance* p3dAnimes;	/// P3DAnimations.fsl
		static const FANLib::FSLInstance* p3dCosmoi;	/// P3DCosmoi.fsl
		static const FANLib::FSLInstance* p3dSound;		/// P3DSound.fsl
		static const FANLib::FSLInstance* p3dMap;		/// P3DMap.fsl
		static const FANLib::FSLInstance* p3dMenu;		/// P3DMenu.fsl

		static const FANLib::FSLInstance* getP3DExtro(); /// P3DExtro.fsl
	};

}

#endif