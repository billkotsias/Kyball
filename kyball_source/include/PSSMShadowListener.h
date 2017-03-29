#pragma once
#ifndef PSSMShadowListener_H
#define PSSMShadowListener_H

#include "OgreSceneManager.h"

namespace P3D
{

	class PSSMShadowListener : public Ogre::SceneManager::Listener
	{
		Ogre::Light *light;
		Ogre::ShadowCameraSetupPtr setup;
		Ogre::Camera *view_camera;        // NOT shadow camera!
		Ogre::SceneManager *sceneMgr;
		mutable int split_index;

	public:

		PSSMShadowListener(Ogre::SceneManager *sm,Ogre::Light *l,Ogre::ShadowCameraSetupPtr s,Ogre::Camera *cam);
		virtual ~PSSMShadowListener() {}
		//virtual void shadowTexturesUpdated(size_t numberOfShadowTextures);
		virtual void shadowTextureCasterPreViewProj(Ogre::Light* light,Ogre::Camera* camera);
		//virtual void shadowTextureReceiverPreViewProj(Ogre::Light* light,Ogre::Frustum* frustum);
		virtual bool sortLightsAffectingFrustum(Ogre::LightList& lightList);
	};
}

#endif
