// ------
// Camera
// ------

#pragma once
#ifndef Camera_H
#define Camera_H

#include "hid\HIDListener.h"
#include "tween\TPlayed.h"
#include <logic\Enabled.h>

namespace Ogre {
	class Camera;
	class SceneNode;
}

namespace P3D {

	class Camera : public HIDListener, public TPlayed, public FANLib::Enabled {
	private:

		Ogre::SceneNode* camera;
		Ogre::SceneNode* target;

		Ogre::Camera* ogreCamera; /// for anyone interested

		double horizontalRot;	/// target SceneNode's rotation; causes camera rotation
		double aHorizontalRot;	/// additional rotation
		double height;			/// camera's height
		double aHeight;			/// additional height
		double distance;		/// camera's distance from target
		double distanceMax;		/// camera's maximum distance from target
		double heightMin;		/// camera's minimum height

		double resetHeight;	/// calculated

		// recalculate and orient camera
		void orientCamera();

		HIDBoss* hid;

	public:

		static const double HORIZONTAL_ROT_MAX;
		static const double ADD_HORIZ_ROT_MAX;
		static const double ADD_HEIGHT_PERCENT;

		static const double CAMERA_RESET_TIME; /// maximum time to return

		// constructor
		// => node to which the Ogre Camera is attached
		//	  camera's target SceneNode
		//	  camera maximum distance from target
		//	  camera minimum height, relative to cube
		//	  actual Ogre Camera
		Camera(Ogre::SceneNode*, Ogre::SceneNode*, double, double, Ogre::Camera*);
		~Camera();

		void reset(); /// reset orientation

		// set main camera orientation
		// => x,y amount to ADD to main orientation
		void moveOrientation(const double& = 0, const double& = 0);

		// set additional orientation
		// => x,y amount to SET to additional orientation [-1...1]
		void setAdditionalOrientation(const double& = 0, const double& = 0);

		// NOT IMPLEMENTED : always track a SceneNode
		// => node = SceneNode to track, or 0 to disable auto-tracking
		void setTracking(Ogre::SceneNode*);

		// overrides
		virtual void onMouseDown();
		virtual void onMouseScroll();
		virtual void onMouseChange();

		virtual inline void disable(FANLib::DisableID id) {
			this->FANLib::Enabled::disable(id);
			acceptMouseEvents(false);
		};

		virtual inline void enable(FANLib::DisableID id) {
			this->FANLib::Enabled::enable(id);
			if (f_Enabled) acceptMouseEvents(true);
		};


		/// getters/setters
		const double& getAddHeight() const		{ return aHeight; };
		Ogre::Camera* getOgreCamera()			{ return ogreCamera; };	

		/// - for tweening :
		const double& getHorizontalRot() const	{ return horizontalRot; };
		const double& getHeight() const			{ return height; };
		void setHorizontalRot(const double& hor) {
			horizontalRot = hor;
			orientCamera();
		};
		void setHeight(const double& hei) {
			height = hei;
			orientCamera();
		};

	};

}

#endif