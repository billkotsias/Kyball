#include "StdAfx.h"
#include "P3DScripts.h"

#include <string>
#include "P3DApp.h"
#include <fsl\FSLParser.h>
#include <fsl\FSLInstance.h>
#include <fsl\FSLEnum.h>

namespace P3D {

	const FANLib::FSLInstance* P3DScripts::p3dLevels = 0;
	//const FANLib::FSLInstance* P3DScripts::p3dChapters = 0;
	const FANLib::FSLInstance* P3DScripts::p3dVars = 0;
	const FANLib::FSLInstance* P3DScripts::p3dAnimes = 0;
	const FANLib::FSLInstance* P3DScripts::p3dCosmoi = 0;
	const FANLib::FSLInstance* P3DScripts::p3dSound = 0;
	const FANLib::FSLInstance* P3DScripts::p3dMap = 0;
	const FANLib::FSLInstance* P3DScripts::p3dMenu = 0;

	const FANLib::FSLInstance* P3DScripts::p3dExtro = 0;

	std::vector<Ogre::Light::LightTypes> P3DScripts::lightTypes;

	void P3DScripts::load() {

		/// parse FSL scripts
		p3dLevels	= parseFile("P3DLevels.fsl");
		//p3dChapters = parseFile("P3DChapters.fsl");
		p3dVars		= parseFile("P3DVars.fsl");
		p3dAnimes	= parseFile("P3DAnimations.fsl");
		p3dCosmoi	= parseFile("P3DCosmoi.fsl");
		p3dSound	= parseFile("P3DSound.fsl");
		p3dMap		= parseFile("P3DMap.fsl");
		p3dMenu		= parseFile("P3DMenu.fsl");

		/// - build FSL "Light" -> Ogre::Light table
		FANLib::FSLEnum* enu = P3DScripts::p3dLevels->getEnum("Light");
		lightTypes.resize(enu->getSize(), Ogre::Light::LT_POINT);
		lightTypes[enu->getValue("point")] = Ogre::Light::LT_POINT;
		lightTypes[enu->getValue("directional")] = Ogre::Light::LT_DIRECTIONAL;
		lightTypes[enu->getValue("spot")] = Ogre::Light::LT_SPOTLIGHT;
	}

	void P3DScripts::unload() {
		delete p3dLevels;
		//delete p3dChapters;
		delete p3dVars;
		delete p3dAnimes;
		delete p3dCosmoi;
		delete p3dSound;
		delete p3dMap;
		delete p3dMenu;
		delete p3dExtro;
	}

	const FANLib::FSLInstance* P3DScripts::getP3DExtro() {
		if (!p3dExtro) {
			p3dExtro = parseFile("P3DExtro.fsl"); /// so, ya finished the game, ya?
		}
		return p3dExtro;
	}

	FANLib::FSLInstance* P3DScripts::parseFile(const char* str) {
		static FANLib::FSLParser parser;	/// <FSLParser>
		static std::string filepath;		/// <static> : maybe it's less expensive to recycle the same string

		filepath = P3DApp::SCRIPTS_PATH + str;
		return parser.parseFile(filepath.c_str());
	}
}