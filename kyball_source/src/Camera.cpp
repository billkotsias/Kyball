#include "StdAfx.h"
#include "game\Camera.h"

#include "P3DApp.h"
#include "SceneBaseCreator.h"
#include "hid\HIDBoss.h"
#include "math\Math.h"

#include "tween\TLinear.h"

#include <OgreAddOns.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>

namespace P3D {

	const double Camera::HORIZONTAL_ROT_MAX	= 85;	/// 85 degrees
	const double Camera::ADD_HORIZ_ROT_MAX = 2.5;	/// degrees, was: 6
	const double Camera::ADD_HEIGHT_PERCENT = 0.03;	/// units, was: 0.06

	const double Camera::CAMERA_RESET_TIME = 0.5;	/// seconds

	Camera::Camera(Ogre::SceneNode* cam, Ogre::SceneNode* node, double dist, double minHeight, Ogre::Camera* ogreCam) : ogreCamera(ogreCam) {

		hid = HIDBoss::getSingletonPtr();

		camera = cam;
		target = node;
		distanceMax = dist;
		distance = distanceMax;
		heightMin = minHeight;

		aHorizontalRot = 0;
		aHeight = 0;
		resetHeight = distanceMax * 0.25;
		reset();
	}

	Camera::~Camera() {
		deleteTweens();
		SceneBaseCreator::getSingletonPtr()->getSceneManager()->destroyCamera(ogreCamera);
	}

	void Camera::reset() {
		deleteTweens();
		horizontalRot = 0;
		height = resetHeight;
		orientCamera();

		/// - 'Enabled'
		disablers.clear();
		f_Enabled = false;
		acceptMouseEvents(true);
	}

	void Camera::onMouseDown() {
		static int x, y;

		if (!hid->isButtonDown(HIDBoss::MOUSE_RIGHT)) return; /// rotate camera if right button is down
		hid->getMouseMove(x, y);
		moveOrientation(x, y);
	}

	void Camera::onMouseScroll() {
		distance -= (double)(hid->getMouseScroll()) * distanceMax * (1./2000.);
		distance = FANLib::Math::bound(distance, distanceMax * 0.85, distanceMax);
		orientCamera();
	}

	void Camera::onMouseChange() {

		/// if right double-click in 0.5 seconds
		if (HIDBoss::getSingletonPtr()->isButtonDClicked(HIDBoss::MOUSE_RIGHT, 10, P3DApp::CYCLES_PER_SEC * 0.5)) {

			/// time depends on "furthest" axis
			double farHor = abs(horizontalRot) / HORIZONTAL_ROT_MAX;
			double farHei = abs(height - resetHeight) / (distanceMax - resetHeight);
			double time = (farHor > farHei) ? farHor * CAMERA_RESET_TIME : farHei * CAMERA_RESET_TIME;

			registerTween(new TLinear<P3D::Camera, double>(
				this, 0, time, &P3D::Camera::getHorizontalRot, &P3D::Camera::setHorizontalRot)
				);

			registerTween(new TLinear<P3D::Camera, double>(
				this, resetHeight, time, &P3D::Camera::getHeight, &P3D::Camera::setHeight)
				);
		}
	}

	void Camera::moveOrientation(const double& x, const double& y) {

		/// horizontal change
		horizontalRot -= x / 6.; /// TODO : calibrate mouse -> rotation conversion (maybe with a deceleration function, e.g y = 2*x^0.8)
		horizontalRot = FANLib::Math::bound(horizontalRot, HORIZONTAL_ROT_MAX);

		/// vertical change
		height += y / 40.; /// bounded in 'orientCamera()'

		orientCamera();
	}

	void Camera::setAdditionalOrientation(const double& x, const double& y) {
		aHorizontalRot = x * ADD_HORIZ_ROT_MAX;
		aHorizontalRot = FANLib::Math::bound(aHorizontalRot, ADD_HORIZ_ROT_MAX);

		aHeight = y; /// not bounded here (should be bounded by caller)

		orientCamera();
	}

	void Camera::orientCamera() {

		/// horizontal orientation
		OgreAddOns::setOrientationYXZ(target, horizontalRot + aHorizontalRot, 0);

		/// vertical orientation
		double minHeight = -(target->getPosition().y * heightMin); /// was: 0.8
		double maxHeight = distance * 0.85; /// was: 0.9
		height = FANLib::Math::bound(height, minHeight, maxHeight);

		Ogre::Vector3 camPos = camera->getPosition();
		camPos.y = height + aHeight * maxHeight * ADD_HEIGHT_PERCENT;
		camPos.z = -sqrt( distance * distance - camPos.y * camPos.y );
		camera->setPosition(camPos);

		//camera->lookAt(target);
	}

}