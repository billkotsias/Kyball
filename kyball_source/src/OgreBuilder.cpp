#include "StdAfx.h"
#include "OgreBuilder.h"

#include "game\GameplayCollection.h"
#include <fsl\FSLArray.h>
#include <fsl\FSLEnum.h>
#include <fsl\FSLClass.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLInstance.h>
#include "visual\ASWDelayed.h"
#include "visual\AnimationStatesList.h"
#include "visual\AnimationStatesCombined.h"
#include "visual\AnimatorElliptical.h"
#include "visual\AnimatorOscillator.h"
#include "visual\AnimatorRotator.h"
#include <core\Utils.h>

#include "FSLToOgre.h"
#include "SceneBaseCreator.h"
#include "P3DScripts.h"
#include "OgreAddOns.h"

namespace P3D {

	std::deque<Ogre::Pass*> OgreBuilder::foggedPasses;

	Ogre::Entity* OgreBuilder::_createEntity(const std::string& entName, const std::string& meshName, const bool& tanVecs, Ogre::SceneNode* node, const bool& shadow)
	{
		static unsigned unnamedRef = 0;
		std::string uniqueName;

		/// create actual entity from mesh
		uniqueName = (entName.empty()) ? "unnamed" + FANLib::Utils::toString(unnamedRef++) : entName;
		Ogre::Entity* entity = SceneBaseCreator::getSingletonPtr()->getSceneManager()->createEntity(uniqueName, meshName);

		/// build tangent vectors
		static unsigned short src, dest;
		Ogre::MeshPtr mesh = entity->getMesh();
		if (tanVecs) {
			if (!mesh->suggestTangentVectorBuildParams(Ogre::VES_TEXTURE_COORDINATES, src, dest))
				mesh->buildTangentVectors(Ogre::VES_TEXTURE_COORDINATES, src, dest);
		}

		/// casts a shadow?
		entity->setCastShadows(shadow);
		if (shadow) mesh->buildEdgeList();

		/// attach to node?
		if (node) node->attachObject(entity);

		return entity;
	}

	Ogre::Entity* OgreBuilder::createEntity(const std::string& entName, const std::string& meshName, const bool& tanVecs,
											const std::string& matName, Ogre::SceneNode* node, const bool& shadow) {

		Ogre::Entity* entity = _createEntity(entName, meshName, tanVecs, node, shadow);

		/// apply material to mesh
		if (matName.size()) entity->setMaterialName(matName);

		return entity;
	}

	Ogre::Entity* OgreBuilder::createEntity(const std::string& entName, const std::string& meshName, const bool& tanVecs,
											const FANLib::FSLArray* matNames, Ogre::SceneNode* node, const bool& shadow) {

		Ogre::Entity* entity = _createEntity(entName, meshName, tanVecs, node, shadow);

		/// apply materials to Entity
		unsigned int subs = entity->getNumSubEntities();
		unsigned int maxAssigns = (matNames->getSize() > subs) ? matNames->getSize() : subs;
		for (unsigned int i = 0; i < maxAssigns; ++i) {
			entity->getSubEntity(i)->setMaterialName(matNames->getCString(i));
		}

		return entity;
	}

	void OgreBuilder::destroyMovable(Ogre::MovableObject *obj) {
		obj->detatchFromParent();
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->destroyMovableObject(obj);
	}

	void OgreBuilder::destroySceneNode(Ogre::SceneNode *node) {
		if (!node) return;
		node->removeAndDestroyAllChildren(); /// this should <never> get used, i.e node should have <no> children!
		if (node->getParent()) node->getParent()->removeChild(node); /// in case of <Forms>, if <AI> gets deleted 1st, the 'pivot' node won't have a parent!
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->destroySceneNode(node);
	}

	std::string OgreBuilder::baseName(std::string name, int num) {
		std::string baseName = name + '#';
		baseName += FANLib::Utils::toString(num); /// <= name + "#NUM"
		return baseName;
	}

	/// ---<-------------------------->---

	void OgreBuilder::setCamera(FANLib::FSLClass *camerafsl, Ogre::Camera* camera) {
		camera->setNearClipDistance( camerafsl->getReal("nearPlane") );
		camera->setFarClipDistance( camerafsl->getReal("farPlane") );
		camera->setFOVy(Ogre::Degree( camerafsl->getReal("fov") ));
	}

	void OgreBuilder::setSkybox(FANLib::FSLClass *skyfsl, Ogre::Camera* camera) {

		/// old skybox material
		static std::string oldName = "";
		static const std::string SKYBOX_GROUP = "SKYBOX";

		/// new material
		std::string matName = skyfsl->getCString("name");

		/// unload old ?! (?!)
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();
		if (sceneMan->isSkyBoxEnabled() && oldName != matName) {
			Ogre::MaterialPtr oldMat = Ogre::MaterialManager::getSingletonPtr()->getByName(oldName);
			if (!oldMat.isNull()) {
				oldMat->unload();
				Ogre::ResourceGroupManager::getSingletonPtr()->unloadResourceGroup(SKYBOX_GROUP);
				oldName = matName;
			}
		}

		if (matName.empty()) {

			sceneMan->setSkyBox(false, "");

		} else {

			double skyboxDistance = camera->getFarClipDistance() * 0.577; /// 1 / sqrt(3)!
			sceneMan->setSkyBox(
				true,
				matName,
				skyboxDistance,
				(skyfsl->getInt("drawFirst") != 0),
				FSLToOgre::arrayToQuaternion( skyfsl->getArray("rot") )
				);
		}
	}

	void OgreBuilder::setSkyplane(FANLib::FSLClass *planefsl) {

		std::string matName = planefsl->getCString("mat");

		if (matName.empty()) {
			SceneBaseCreator::getSingletonPtr()->getSceneManager()->setSkyPlane(false, Ogre::Plane(), "");
			return;
		}

		Ogre::Vector3 norm = FSLToOgre::vector3ToVector3(planefsl->getClass("norm"));
		double planeDist = planefsl->getReal("dist");
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->setSkyPlane(
			true, Ogre::Plane(norm, planeDist), matName, planefsl->getReal("scale"), planefsl->getReal("tile"), true,
			planefsl->getReal("bow"), planefsl->getReal("xseg"), planefsl->getReal("yseg"));
	}

	void OgreBuilder::setFog(FANLib::FSLClass *fogfsl, const Ogre::ColourValue& backgroundColour) {

		unsetFog();

		if (fogfsl->getInt("set")) {
			double expDensity = fogfsl->getReal("exp");
			setFogToMaterials(backgroundColour, expDensity);
			SceneBaseCreator::getSingletonPtr()->getSceneManager()->setFog(Ogre::FOG_EXP, backgroundColour, expDensity); /// set for all others
		}
	}
	void OgreBuilder::setFogToMaterials(const Ogre::ColourValue &backgroundColour, const Ogre::Real &expDensity) {
		std::set<Ogre::MaterialPtr> sceneMaterials;
		OgreAddOns::getMaterialsIn(sceneMaterials, SceneBaseCreator::getSingletonPtr()->getSceneManager()->getRootSceneNode());
		for (std::set<Ogre::MaterialPtr>::iterator it = sceneMaterials.begin(); it != sceneMaterials.end(); ++it) {
			setFogToMaterial( (*it), backgroundColour, expDensity );
		}
	}
	void OgreBuilder::setFogToMaterial(Ogre::MaterialPtr& material, const Ogre::ColourValue &backgroundColour, const Ogre::Real &expDensity)
	{
		Ogre::ColourValue halfColour = backgroundColour / 2;

		Ogre::Technique* technique;
		if ( !(technique = material->getBestTechnique()) ) {
			material->load();
			technique = material->getBestTechnique();
		}

		Ogre::Technique::PassIterator it2 = technique->getPassIterator();
		bool hasFogOverride = false;
		while (it2.hasMoreElements()) {
			Ogre::Pass* pass = it2.getNext();
			if (pass->getFogOverride()) {
				hasFogOverride = true;
				break;
			}
		}
		if (hasFogOverride) return; /// technique has fog-override pass, don't touch

		/// try to guess what to set by the number of passes and their type
		int numPasses = technique->getNumPasses();
		Ogre::Pass* pass1;
		Ogre::Pass* pass2;

		/// <TODO Kyball2> : this should be turned into an algorithm which automatically calculates the fog to set in each pass for correct result!!!
		/// NOTE : SIMPLY Set <BLACK FOG> for every pass which is <ADD>ED!
		/// NOTE : For mutliple passes, maybe, just maybe, <expDensity> should be decreased!
		if ( numPasses == 1 )
		{
			pass1 = technique->getPass(0);
			if (pass1->getSourceBlendFactor() == Ogre::SBF_ONE &&
				pass1->getDestBlendFactor() == Ogre::SBF_ONE)
			{
				foggedPasses.push_back(pass1);
				pass1->setFog(true, Ogre::FOG_EXP, Ogre::ColourValue::Black, expDensity); /// <BLACK> FOG FOR SINGLE ADDED PASSES <!!!>
			}
		} else
		if ( numPasses >= 2 )
		{
			pass1 = technique->getPass(0);
			pass2 = technique->getPass(1);
			if (pass1->getSourceBlendFactor() == Ogre::SBF_ONE &&
				pass1->getDestBlendFactor() == Ogre::SBF_ZERO &&
				pass2->getSourceBlendFactor() == Ogre::SBF_ONE &&
				pass2->getDestBlendFactor() == Ogre::SBF_ONE)
			{
				foggedPasses.push_back(pass1);
				foggedPasses.push_back(pass2);
				pass1->setFog(true, Ogre::FOG_EXP, halfColour, expDensity); /// HALF FOG FOR EACH OF THE 2 PASSES
				pass2->setFog(true, Ogre::FOG_EXP, halfColour, expDensity); /// HALF FOG FOR EACH OF THE 2 PASSES
			}

			//if (numPasses >= 3)
			//{
			//	Ogre::Pass* pass3 = technique->getPass(2);
			//}
		}
	}

	void OgreBuilder::unsetFog() {
		for (int i = foggedPasses.size() - 1; i >= 0; --i) {
			Ogre::Pass* pass = foggedPasses.at(i);
			pass->setFog(false);
		}
		foggedPasses.clear();

		SceneBaseCreator::getSingletonPtr()->getSceneManager()->setFog(Ogre::FOG_NONE);
	}

	void OgreBuilder::createNodes(FANLib::FSLArray* arrayfsl, Ogre::SceneNode* parent, GameplayCollection* collection) {
		for (unsigned int i = 0; i < arrayfsl->getSize(); ++i) {
			OgreBuilder::createNode(arrayfsl->getClass(i), parent, collection);
		}
	}

	Ogre::SceneNode* OgreBuilder::createNode(FANLib::FSLClass *nodeClass, Ogre::SceneNode* parent, GameplayCollection* collection)
	{
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();

		/// create node
		std::string nodeName = nodeClass->getCString("n");
		Ogre::SceneNode* node;
		if (parent) {
			if (nodeName.empty()) {
				node = parent->createChildSceneNode();
			} else {
				node = parent->createChildSceneNode(nodeName);
			}
		} else {
			/// this may as well never be used!
			if (nodeName.empty()) {
				node = sceneMan->createSceneNode();
			} else {
				node = sceneMan->createSceneNode(nodeName);
			}
		}

		collection->nodes.push_back(node);
		node->setPosition(FSLToOgre::vector3ToVector3(nodeClass->getClass("pos")));
		node->setScale(FSLToOgre::vector4ToVector3(nodeClass->getClass("scale")));
		node->setOrientation(FSLToOgre::arrayToQuaternion(nodeClass->getArray("rot")));

		/// all created <MovableObjects> are kept in the 'collection' to <be destroyed> later altogether
		/// - attach <entities>
		FANLib::FSLArray* entities = nodeClass->getArray("entities");
		for (unsigned int i = 0; i < entities->getSize(); ++i) {

			FANLib::FSLClass* fslEntity = entities->getClass(i);
			FANLib::FSLClass* obj = fslEntity->getClass("obj");

			/// -- create Ogre Entity
			Ogre::Entity* entity = OgreBuilder::createEntity(
				fslEntity->getCString("name"),
				obj->getCString("mesh"),
				(obj->getInt("buildTanVec") != 0),
				obj->getArray("material"),
				node,
				obj->getInt("shadow")
				);
			collection->storeMovable(entity);

			/// <TODO> : forget it...
			///// -- check for entity bone attachments
			//FANLib::FSLArray* boneAttachments = fslEntity->getArray("attachments");
			//for (unsigned int j = 0; j < boneAttachments->getSize(); ++j) {
			//	FANLib::FSLClass* fslBoneAttach = boneAttachments->getClass(j);
			//	entity->attachObjectToBone(
			//		fslBoneAttach->getCString("bone"),
			//		OgreBuilder::createEntity;
			//}

			/// -- check for <mesh animations>
			FANLib::FSLArray* fslAnims = fslEntity->getArray("anims");
			if (fslAnims->getSize())
			{
				std::vector<AnimationStateWrapper*> anims;
				for (unsigned int i = 0; i < fslAnims->getSize(); ++i)
				{
					FANLib::FSLClass* fslAnim = fslAnims->getClass(i);
					Ogre::AnimationState* animState = entity->getAnimationState( fslAnim->getCString("n") );
					AnimationStateWrapper* anim = new ASWDelayed(animState, fslAnim->getReal("s"), fslAnim->getReal("d"));
					anims.push_back(anim);
				}

				FANLib::FSLClass* fslasCommon = fslEntity->getClass("asCommon");
				double startPos = fslasCommon->getReal("pos");
				AnimationStatesList::LoopType loopType = (AnimationStatesList::LoopType)( fslasCommon->getInt("loop") );
				switch (loopType)
				{
					case AnimationStatesList::combined:
						collection->storeVisual( new AnimationStatesCombined(anims) ); /// <TODO> : implement 'startPos' for "combined" type
						break;
					default:
						collection->storeVisual( new AnimationStatesList(anims, loopType, startPos) );
						break;
				}				
			}
			entity->_initialise();
		}

		/// - attach <cameras>
		FANLib::FSLArray* cameras = nodeClass->getArray("cameras");
		for (unsigned int i = 0; i < cameras->getSize(); ++i) {

			FANLib::FSLClass* fslCamera = cameras->getClass(i);
			char* camName = fslCamera->getCString("name");
			Ogre::Camera* camera = (Ogre::Camera*)collection->storeMovable(sceneMan->createCamera(camName));
			setCamera(fslCamera, camera);
			
			/// attach camera to this node
			node->attachObject(camera);
		}

		/// - attach <lights>
		FANLib::FSLArray* lights = nodeClass->getArray("lights");
		for (unsigned int i = 0; i < lights->getSize(); ++i) {
			FANLib::FSLClass* fslLight = lights->getClass(i);
			char* lightName = fslLight->getCString("name");

			Ogre::Light* light = (Ogre::Light*)collection->storeMovable(sceneMan->createLight(lightName));
			light->setType(P3DScripts::lightTypes[fslLight->getInt("type")]);
			light->setDiffuseColour(FSLToOgre::rgbToColourValue(fslLight->getClass("diffuse")));
			light->setSpecularColour(FSLToOgre::rgbToColourValue(fslLight->getClass("specular")));
			light->setDirection(FSLToOgre::vector3ToVector3(fslLight->getClass("direction")));
			//light->setShadowFarDistance(100);

			/// TODO : spotRange (EASY-PEASY!)

			FANLib::FSLClass* fslAtten = fslLight->getClass("attenuation"); /// attenuation
			light->setAttenuation( fslAtten->getReal("range"), fslAtten->getReal("constant"), fslAtten->getReal("linear"), fslAtten->getReal("quad") );

			light->setVisible(fslLight->getInt("on") != 0);
			light->setCastShadows(fslLight->getInt("shadows") != 0);

			/// attach light to this node
			node->attachObject(light);
		}

		/// - create <Ogre node animation>
		FANLib::FSLArray* fslnanims = nodeClass->getArray("nanims");
		if (fslnanims->getSize())
		{
			FANLib::FSLClass* fslnaCommon = nodeClass->getClass("naCommon");
			bool autoAnims = fslnaCommon->getInt("auto") != 0;
			double animsSpeed = fslnaCommon->getReal("speed");
			std::vector<AnimationStateWrapper*> nodeAutoAnims;

			for (unsigned int i = 0; i < fslnanims->getSize(); ++i)
			{
				static unsigned unnamedRef = 0;

				FANLib::FSLClass* fslanim = fslnanims->getClass(i); /// 'NodeAnim' FSL Class
				std::string uniqueName = fslanim->getCString("name");
				if ( uniqueName.empty() ) uniqueName = "unnamedNodeAnim" + FANLib::Utils::toString(unnamedRef++);

				double timeScale = fslanim->getReal("timeScale");
				Ogre::Animation* ogreAnim = sceneMan->createAnimation( uniqueName, fslanim->getReal("length") * timeScale );
				ogreAnim->setInterpolationMode(Ogre::Animation::IM_SPLINE);
				ogreAnim->setRotationInterpolationMode(Ogre::Animation::RIM_SPHERICAL);
				Ogre::NodeAnimationTrack* ogreTrack = ogreAnim->createNodeTrack(0, node);
				ogreTrack->setUseShortestRotationPath(true);

				FANLib::FSLClass* keysCont = fslanim->getClass("keys");
				FANLib::FSLArray* keys = keysCont->getArray("k");
				for (unsigned int j = 0; j < keys->getSize(); ++j) {
					FANLib::FSLClass* key = keys->getClass(j);
					Ogre::TransformKeyFrame* ogreKey = ogreTrack->createNodeKeyFrame(key->getReal("time") * timeScale);
					ogreKey->setScale( FSLToOgre::vector4ToVector3(key->getClass("scale")) );
					ogreKey->setRotation( FSLToOgre::arrayToQuaternion(key->getArray("rot")) );
					ogreKey->setTranslate( FSLToOgre::vector3ToVector3(key->getClass("trans")) );
				}
				// NOTE : fix bug with "ogreTrack->setUseShortestRotationPath", maybe OK in future OGRE versions...
				for (unsigned int j = 1; j < keys->getSize(); ++j) {
					ogreTrack->getNodeKeyFrame(j)->setRotation(
						Ogre::Quaternion::Slerp(1., ogreTrack->getNodeKeyFrame(j-1)->getRotation(), ogreTrack->getNodeKeyFrame(j)->getRotation(), true)
						);
				}
				ogreTrack->optimise();
				collection->storeAnimation(ogreAnim); /// <NOTE> : this means that any animation states will be destroyed altogether later

				if (autoAnims)
				{
					Ogre::AnimationState* animState = sceneMan->createAnimationState( ogreAnim->getName() );
					AnimationStateWrapper* nodeAutoAnim = new ASWDelayed(animState, animsSpeed, fslanim->getReal("delay")); /// no need for per-anim speed cause there's "timeScale"
					nodeAutoAnims.push_back(nodeAutoAnim);
				}
			}

			/// - automatically play this Node's animations?
			if (autoAnims) {
				double startPos = fslnaCommon->getReal("start");
				AnimationStatesList::LoopType loopType = (AnimationStatesList::LoopType)( fslnaCommon->getInt("loop") );
				switch (loopType)
				{
					case AnimationStatesList::combined:
						collection->storeVisual( new AnimationStatesCombined(nodeAutoAnims) ); /// <TODO> : implement 'startPos' for "combined" type
						break;
					default:
						collection->storeVisual( new AnimationStatesList(nodeAutoAnims, loopType, startPos) );
						break;
				}
			}
		}

		/// is node initially visible?
		node->setVisible(nodeClass->getInt("visible") != 0);

		/// - attach <children nodes>
		FANLib::FSLArray* nodes = nodeClass->getArray("nodes");
		for (unsigned int i = 0; i < nodes->getSize(); ++i) {
			createNode(nodes->getClass(i), node, collection); /// recursive node creation
		}

		/// - create <Animators>; NOTE : MUST be created <AFTER> children nodes, at 'Animator' may require them!
		FANLib::FSLArray* animos = nodeClass->getArray("animators");
		for (unsigned int i = 0; i < animos->getSize(); ++i) {
			collection->storeVisual(createAnimator(animos->getClass(i), node));
		}

		return node;
	}

	Visual* OgreBuilder::createAnimator(FANLib::FSLClass *fslAnim, Ogre::SceneNode *node) {

		// code executed only once - assumes P3DScripts have been loaded already!
		static const FANLib::FSLEnum* fslEnum = P3DScripts::p3dCosmoi->getEnum("Animator");
		static const int ELLIPSE = fslEnum->getValue("ellipse");
		static const int OSCILLATE = fslEnum->getValue("oscillate");
		static const int ROTATE = fslEnum->getValue("rotate");
		// code executed only once

		int type = fslAnim->getInt("t");
		if (type == ELLIPSE) {
			return new AnimatorElliptical(
				node, fslAnim->getReal("elMa"), fslAnim->getReal("elMi"), fslAnim->getReal("elFi"), fslAnim->getReal("elAngle"),
				fslAnim->getReal("s"), fslAnim->getReal("start")
				);
		} else if (type == OSCILLATE) {
			return new AnimatorOscillator(
				node, FSLToOgre::vector3ToVector3(fslAnim->getClass("osAx")), fslAnim->getReal("osWi"), fslAnim->getInt("osLocal") != 0,
				fslAnim->getReal("s"), fslAnim->getReal("start")
				);
		} else if (type == ROTATE) {
			return new AnimatorRotator(
				node, FSLToOgre::vector3ToVector3(fslAnim->getClass("roAx")),
				fslAnim->getReal("s"), fslAnim->getReal("start")
				);
		}

		return 0;
	}

	Ogre::Entity* OgreBuilder::createSphereEntity(const std::string& entName, const int& nRings, const int& nSegments, const bool& invertNormals, const std::string& matName, Ogre::SceneNode* parent, const bool& shadow)
	{
		static const float RADIUS_DEFAULT = 1.f;

		std::string sphereMeshName = FANLib::Utils::toString(nRings) + "x" + FANLib::Utils::toString(nSegments) + "sphereMesh";
		Ogre::SceneManager* sceneMan = SceneBaseCreator::getSingletonPtr()->getSceneManager();
		if (!Ogre::ResourceGroupManager::getSingletonPtr()->resourceExists(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, sphereMeshName))
		{
			OgreAddOns::createSphereMesh(sceneMan, sphereMeshName, RADIUS_DEFAULT, nRings, nSegments, invertNormals);
		}

		return createEntity(entName, sphereMeshName, false, matName, parent, shadow);
	}
}