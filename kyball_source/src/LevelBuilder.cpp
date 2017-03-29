#include "StdAfx.h"
#include "game\LevelBuilder.h"

#include "menu\Player.h"

#include "SceneBaseCreator.h"
#include "OgreBuilder.h"

#include <fsl\FSLArray.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>
#include "P3DScripts.h"
#include "FSLToOgre.h"

#include "game\GameplayBase.h"
#include "game\GameplayObject.h"
#include "game\Level.h"
#include "game\cannon\Aim.h"
#include "game\cannon\BasicBallCreator.h"
#include "game\cannon\Cannon.h"
#include "game\Camera.h"
#include "game\form\BallState.h"
#include "game\form\BallForm.h"
#include "game\form\BobForm.h"
#include "game\form\WallForm.h"
#include "game\form\SphereForm.h"
#include "game\form\CylinderForm.h"
#include "game\form\ClosedCube.h"
#include "game\form\OpenCube.h"
#include "game\ai\BounceAI.h"
#include "game\ai\BallAI.h"
#include "game\ai\BobAI.h"
#include "game\ai\BroAI.h"
#include "game\ai\StickyAI.h"
#include "game\ai\DestroyAI.h"
#include "game\event\EventManager.h"
#include "game\rule\RuleManager.h"
#include "game\rule\MatchThree.h"

#include "visual\Ambience.h"
#include "visual\EnergyChange.h"
#include "visual\PreviewLevel.h"

#include "gui\HintBox.h"

#include "OgreAddOns.h"
#include "Queue.h"
#include "Particles.h"

#include <stdLib.h>

//#include <OgreColourValue.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>
//#include <OgreLight.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreVector3.h>
#include <OgreCamera.h>

#include <core\Utils.h>

//#ifdef _DEBUG
//	#include <core\Log.h>
//	#include <core\Utils.h>
//#endif

#include "game\net\Net.h"

namespace P3D {

	LevelBuilder::LevelBuilder() {
	}

	LevelBuilder::~LevelBuilder() {
	}

	// build a level
	Level* LevelBuilder::build(unsigned levelNum, GameplayBase* base, Queue* queue, HintBox* hintBox) {

		// code executed only once - assumes P3DScripts have been loaded already!
		static const FANLib::FSLEnum* lsEnum = P3DScripts::p3dLevels->getEnum("ls");
		static const int CUBE = lsEnum->getValue("cube");
		static const int CYLINDER = lsEnum->getValue("cyl");
		static const int SPHERE = lsEnum->getValue("sph");
		static const int WALL = lsEnum->getValue("wall");
		static const int PLANE = lsEnum->getValue("plan");

		static const FANLib::FSLEnum* lsaiEnum = P3DScripts::p3dLevels->getEnum("lsai");
		static const int STICKY = lsaiEnum->getValue("sticky");
		static const int BOUNCE = lsaiEnum->getValue("bounce");
		static const int DESTROY = lsaiEnum->getValue("destroy");
		// code executed only once

		Level* levelStruct = new Level(); /// created level must be deleted by the requirer
		base->insertGameplayObjects(&(levelStruct->gameplayObjects)); /// register 'level's <gameplayObjects>

		/// read <Level>
		FANLib::FSLClass* level = P3DScripts::p3dLevels->getRoot()->getArray(base->getCosmos().c_str())->getClass(levelNum);

		/// - Level <Rules>
		RuleManager* ruleManager = base->getRuleManager();
		ruleManager->clearRules(); /// !!!
		ruleManager->addRule(new MatchThree(base)); /// <TODO> : read level <rules> from script

		/// - Level <BallCreator>
		BallCreator* ballCreator = new BasicBallCreator(base);		/// <TODO> : support different types..?
		ballCreator->setIncomingBalls(level->getArray("incoming"));	/// set non-random "incoming" balls
		base->getCannon()->attachBallCreator(ballCreator);
		base->getCannon()->attachAim(new Aim(base, level->getInt("aimLength")));

		/// - Level <decoration>
		SceneBaseCreator* sceneBase = SceneBaseCreator::getSingletonPtr();
		Ogre::SceneManager* sceneMan = sceneBase->getSceneManager();

		/// - sound ambience
		FANLib::FSLArray* ambiencesfsl = level->getArray("sounds");
		for (int i = ambiencesfsl->getSize() - 1; i >= 0; --i)
		{
			FANLib::FSLClass* ambiencefsl = ambiencesfsl->getClass(i);
			FANLib::FSLArray* soundNamesfsl = ambiencefsl->getArray("n");
			std::vector<std::string> soundNames( soundNamesfsl->getSize() );
			for (unsigned int j = 0; j < soundNamesfsl->getSize(); ++j) {
				soundNames.at(j) = base->getCosmos() + "\\" + soundNamesfsl->getCString(j);
			}
			// it would be better if Ambience was allowed to self-destruct in some way...
			levelStruct->storeVisual(
				new Ambience(
					soundNames, sceneMan->getRootSceneNode(),
					FSLToOgre::vector3ToVector3(ambiencefsl->getClass("c")), ambiencefsl->getReal("i"), ambiencefsl->getReal("o"), ambiencefsl->getReal("h"),
					ambiencefsl->getReal("sInterval"), ambiencefsl->getReal("sRand"), ambiencefsl->getInt("pause") != 0 ) );
		}

		/// - ambient and background colours
		Ogre::ColourValue backgroundColour = FSLToOgre::rgbToColourValue(level->getClass("background"));
		sceneBase->getViewport()->setBackgroundColour(backgroundColour);
		sceneMan->setAmbientLight(FSLToOgre::rgbToColourValue(level->getClass("ambient")));
		/// - sky materials
		OgreBuilder::setSkybox(level->getClass("skybox"), base->getCamera()->getOgreCamera());
		OgreBuilder::setSkyplane(level->getClass("plane"));

		/// - marks of <energy-changes>
		FANLib::FSLArray* fslChanges = level->getArray("changes");
		for (unsigned int i = 0; i < fslChanges->getSize(); ++i) {
			FANLib::FSLClass* fslChange = fslChanges->getClass(i);
			new EnergyChange(sceneMan->getRootSceneNode(), FSLToOgre::vector3ToVector3(fslChange->getClass("pos")), FSLToOgre::vector4ToVector3(fslChange->getClass("scale")),
				fslChange->getReal("ttl"), fslChange->getReal("rnd"), FSLToOgre::arrayToQuaternion(fslChange->getArray("rot")), i == 0);
		}

		// NOTE: FOG WAS HERE

		/// nodes (big! includes level-decoration objects, lights, cameras...)
		OgreBuilder::createNodes(level->getArray("nodes"), sceneMan->getRootSceneNode(), levelStruct);

		/// - ambient particles
		/// BULLSHIT : <TODO> : THIS SHOULD BE IN OgreBuilder (part of Node!!!)
		FANLib::FSLArray* fslParticles = level->getArray("particles");
		for (unsigned int i = 0; i < fslParticles->getSize(); ++i) {
			FANLib::FSLClass* fslParticle = fslParticles->getClass(i);
			std::string particleName = fslParticle->getCString("name");
			std::string parentName = fslParticle->getCString("parent");
			Ogre::SceneNode* parentNode = (parentName.length()) ? sceneMan->getSceneNode(parentName) : sceneMan->getRootSceneNode();
			Particles* particles = new Particles(
				particleName, parentNode,
				FSLToOgre::vector3ToVector3(fslParticle->getClass("pos")), base->getRootNode()->getScale().x, fslParticle->getReal("ff"), fslParticle->getInt("shadows") != 0);
			levelStruct->addParticles( particles ); /// will be self-destructed
		}

		/// gameplay objects
		Net* net = base->getNet();
		net->reset();

		/// - level balls
		{
			FANLib::FSLArray* fslBalls = level->getArray("balls");
			for (unsigned int i = 0; i < fslBalls->getSize(); ++i) {
				/// - read ball properties
				FANLib::FSLArray* ball = fslBalls->getArray(i);
				int x = ball->getInt(0);
				int y = ball->getInt(1);
				int z = ball->getInt(2);
				BallState::BallType ballType = BallState::ballTypes[ball->getInt(3)];
				/// - create ball
				GameplayObject* obj = levelStruct->storeGameplayObject(new GameplayObject(base));
				Ogre::Vector3 objectPos = OgreAddOns::worldToLocalParent(base->getRootNode(), &(net->netNodeToWorld(x,y,z)) );
				BallAI* ballAI;
				BallForm* ballForm;
				if (ballType < BallState::BOB) {
					ballAI = new BallAI(obj, BallAI::STUCK, base->getRootNode(), objectPos);
					ballForm = new BallForm(base, base->getBBSet(GameplayBase::BALLS), ballType, true);
				} else if (ballType == BallState::BOB) {
					ballAI = new BobAI(obj, base->getRootNode(), objectPos);
					ballForm = new BobForm(base, base->getBBSet(GameplayBase::BOBS), BallState::BOB);
				} else if (ballType == BallState::BRO) {
					ballAI = new BroAI(obj, base->getRootNode(), objectPos);
					ballForm = new BobForm(base, base->getBBSet(GameplayBase::BOBS), BallState::BRO);
				}
				ballAI->setVelocity(Ogre::Vector3::ZERO);
				obj->setAI(ballAI);
				obj->setForm(ballForm);
			}
		}

		/// set level <depth> from script
		Ogre::Vector3 cubeSize = base->getCubeSize(); /// this is standard for a cosmos
		int cubeSizeZ = level->getInt("z");
		base->levelZ = cubeSizeZ;

		/// - create <container>
		if (cubeSizeZ > 0) {
			GameplayObject* obj = levelStruct->storeGameplayObject( new GameplayObject(base) );
			obj->setAI(new BounceAI(obj, base->getRootNode(), Ogre::Vector3::ZERO, false));
			obj->setForm(new OpenCube(base, cubeSizeZ));
		}

		/// - advanced level structure
		{
			FANLib::FSLArray* fslStructs = level->getArray("struct");
			for (unsigned int i = 0; i < fslStructs->getSize(); ++i) {

				GameplayObject* obj = levelStruct->storeGameplayObject( new GameplayObject(base) );
				FANLib::FSLClass* fslStruct = fslStructs->getClass(i);

				/// create & set AI & corresponding 'Form' material
				AIPosVel* objAI;
				std::string formMaterial;
				bool mover = fslStruct->getInt("m") != 0;
				if (fslStruct->getInt("a") == STICKY) {
					objAI = new StickyAI(obj, base->getRootNode(), Ogre::Vector3::ZERO, mover);
					formMaterial = base->getCosmos() + "_cube_back";
				} else if (fslStruct->getInt("a") == BOUNCE) {
					objAI = new BounceAI(obj, base->getRootNode(), Ogre::Vector3::ZERO, mover);
					formMaterial = base->getCosmos() + "_block";
				} else if (fslStruct->getInt("a") == DESTROY) {
					objAI = new DestroyAI(obj, base->getRootNode(), Ogre::Vector3::ZERO, mover);
					formMaterial = base->getCosmos() + "_cube_back"; /// debug
				}
				objAI->setOrientation(FSLToOgre::arrayToQuaternion(fslStruct->getArray("rot")));
				obj->setAI(objAI);

				bool visible = fslStruct->getInt("v") != 0; /// should be supported by all 'Form's!

				/// create & set Form
				Form* objForm;
				if (fslStruct->getInt("t") == WALL) { /// Cube's Wall

					/// only <1> allowed per-base!
					objAI->setPosition(Ogre::Vector3(cubeSize.x / 2., 0, (net->getZDiff() * cubeSizeZ))); /// special position for 'Wall'
					objForm = new WallForm(std::string("CubeWall") + base->getID(), formMaterial, cubeSize.x, cubeSize.y, net->getHalfGridSize(), visible);

				} else {

					int x = fslStruct->getInt("x");
					int y = fslStruct->getInt("y");
					int z = fslStruct->getInt("z");
					objAI->setPosition( OgreAddOns::worldToLocalParent( base->getRootNode(), &(net->netNodeToWorld(x,y,z)) ) );
					std::string formName = base->getID() + std::string("Struct") + FANLib::Utils::toString(i);

					if (fslStruct->getInt("t") == CUBE) { /// a cube

						int w = fslStruct->getInt("w");
						int h = fslStruct->getInt("h");
						int d = fslStruct->getInt("d");
						objForm = new ClosedCube(formName, formMaterial, w, h, d, visible);

					} else if (fslStruct->getInt("t") == SPHERE) { /// a sphere

						double diameter = fslStruct->getInt("w");
						objForm = new SphereForm(base, formName, formMaterial, diameter, visible);

					} else if (fslStruct->getInt("t") == CYLINDER) { /// a cylinder

						double height = fslStruct->getInt("h");
						double diameter = fslStruct->getInt("w");
						objForm = new CylinderForm(formName, formMaterial, height, diameter, visible);

					} else if (fslStruct->getInt("t") == PLANE) { /// a plane

						double w = fslStruct->getInt("w");
						double h = fslStruct->getInt("h");
						objForm = new WallForm(formName, formMaterial, w, h, net->getHalfGridSize(), visible);

					}
				}
				obj->setForm(objForm);

				if (fslStruct->getInt("l")) objAI->link(); /// must move along other linked objects?
			}
		}

		/// - create <Lose-Wall>
		{
			//GameplayObject* wall = levelStruct->storeGameplayObject( new GameplayObject(base) );
			//AI* wallAI = new AIPos(wall, base->getRootNode(), Ogre::Vector3(0, 0, (net->getZDiff() * 0) - net->getHalfGridSize() - 0.0));
			//Form* wallForm = new WallForm(std::string("LoseWall") + base->getID(), base->getCosmos(), base->getEventManager(), cubeSize.x, cubeSize.y, 180);
			//wall->setAI(wallAI);
			//wall->setForm(wallForm);
		}

		/// <initialization> of various level components
		/// - initialize <Cannon> etc... *AFTER* level creation!
		{
			Cannon* cannon = base->getCannon();
			cannon->reset();
			cannon->setMaximumDeviation(level->getReal("maxCannonDeviation"));
		}
		base->getCamera()->reset();
		/// - initialize <Rules>
		ruleManager->initRules();

		/// show "preview" camera
		FANLib::FSLArray* previews = level->getArray("previews");
		for (unsigned int i = 0; i < previews->getSize(); ++i) {
			FANLib::FSLClass* fslPreview = previews->getClass(i);
			PreviewLevel* preview = new PreviewLevel(fslPreview->getCString("cam"), fslPreview->getReal("t"));
			queue->pushBack(preview, &PreviewLevel::start);
		}

		/// read and set hintsBox
		std::string hint = level->getClass("hint")->getCString("h");
#ifdef _DEPLOY
		if (!base->player->hasFinishedGame() && hint.size()) {
#else
		if (hint.size()) {
#endif
			hintBox->setText(hint);
			hintBox->animal = base->getAnimal();
			queue->pushBack(hintBox, &HintBox::open);
		}

		/// - <FOG> must be last, as it must be hacked-in the scene <"stupid" Ogre>
		// TODO : Build an array of materials created in Cosmos/Level builders and pass it in below call as param!
		OgreBuilder::setFog(level->getClass("fog"), backgroundColour);

		return levelStruct;
	}
}