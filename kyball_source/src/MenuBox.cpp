#include "StdAfx.h"
#include "gui\MenuBox.h"

#include "P3DWindow.h"
#include "tween\TLinear.h"
#include "hid\HIDBoss.h"

namespace P3D {

	const double MenuBox::APPEAR_TIME = 0.2;
	const double MenuBox::DISAPPEAR_TIME = 0.1;
	const Ogre::Vector2 MenuBox::OVER_SIZE = Ogre::Vector2(1.35, 1.65);	
	const double MenuBox::OVER_TIME = 0.05;	
	const int MenuBox::CLICK_DIST = 4;

	MenuBox::MenuBox(const std::string& name, const std::string& clonePrefix) :
		OverlayBox(name, clonePrefix), FANLib::MCBSystem(_last), belowMouse(false), defaultSize(1.,1.), delay(0)
	{
	}

	void MenuBox::open() {

		/// non-changing properties
		setFontHeight( borderSize.x ); /// <NOTE> : this sucks big time!!!

		/// tweened properties
		setSize(Ogre::Vector2(0,0)); /// initial size : set AFTER border size
		setScale(Ogre::Vector2(0,0));

		TPlaylist* list;

		// show
		ogreBox->show();
		shadowText->setVisible(true);
		editText->setVisible(true);

		/// list 1 : appear
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2::UNIT_SCALE, APPEAR_TIME / 2., &OverlayBox::getScale, &OverlayBox::setScale, delay)
			);
		registerList(list);

		/// list 2 : unfold
		list = new TPlaylist();
		list->id = OPENING;
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, defaultSize, APPEAR_TIME / 2., &OverlayBox::getSize, &OverlayBox::setSize, delay + APPEAR_TIME / 2.)
			);
		registerList(list);
	}

	void MenuBox::close() {

		acceptMouseEvents(false);

		TPlaylist* list;

		/// list 1 : fold
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2::ZERO, APPEAR_TIME / 2., &OverlayBox::getSize, &OverlayBox::setSize, delay)
			);
		registerList(list);

		/// list 2 : disappear
		list = new TPlaylist();
		list->id = CLOSED;
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2::ZERO, APPEAR_TIME / 2., &OverlayBox::getScale, &OverlayBox::setScale, delay + APPEAR_TIME / 2.)
			);
		registerList(list);
	}

	void MenuBox::playlistFinished(TPlaylist* list) {

		this->TPlayed::playlistFinished(list);

		if (list->id == OPENING) {
			acceptMouseEvents(true);
			onMouseMove();
		} else if (list->id == CLOSED) {
			callBackSafe(CLOSED);
			//if (onCloseF) onCloseF(onCloseO, this);
		}
	}

	bool MenuBox::checkBelowMouse() {
		static int x,y;
		static Ogre::Vector2 topLeft;
		static Ogre::Vector2 bottomRight;
		static Ogre::Vector2 temp;

		temp = size / 2. + borderSize;
		topLeft = position - temp;
		topLeft.x /= P3DWindow::getPixelRatio();
		bottomRight = position + temp;
		bottomRight.x /= P3DWindow::getPixelRatio();

		HIDBoss::getSingletonPtr()->getMousePosition(x,y);
		temp.x = x;
		temp.y = y;

		if (topLeft < temp && bottomRight > temp) {
			return true;
		}

		return false;
	}

	void MenuBox::onMouseMove() {

		if (belowMouse != checkBelowMouse()) {

			/// delete possible tweening but keep current size
			Ogre::Vector2 restoreSize = size;
			deleteTweens();
			setSize(restoreSize);

			belowMouse = !belowMouse;
			if (belowMouse) {

				this->registerTween(new TLinear<OverlayBox, Ogre::Vector2>(
					this, defaultSize * OVER_SIZE, OVER_TIME, &OverlayBox::getSize, &OverlayBox::setSize)
					);
			} else {

				this->registerTween(new TLinear<OverlayBox, Ogre::Vector2>(
					this, defaultSize, OVER_TIME, &OverlayBox::getSize, &OverlayBox::setSize)
					);
			}
		}
	}

	void MenuBox::onMouseChange() {
		if (belowMouse && HIDBoss::getSingletonPtr()->isButtonClicked(HIDBoss::MOUSE_LEFT, CLICK_DIST)) {
			//if (onClickF) onClickF(onClickO, this);
			callBackSafe(CLICKED);
		}
	}
}