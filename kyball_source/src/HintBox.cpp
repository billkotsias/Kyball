#include "StdAfx.h"
#include "gui\HintBox.h"
#include "Queue.h"

#include "P3DWindow.h"
#include "tween\TLinear.h"
#include "hid\HIDBoss.h"
#include "visual\VisualBoss.h"
#include "core\String.h"
#include "game\animal\Animal.h"

namespace P3D {

	const double HintBox::APPEAR_TIME = 0.3;
	const double HintBox::HOLD_TIME = 0.2;
	const double HintBox::UNFOLD_TIME = 0.33;
	const double HintBox::UNFOLD_TEXT_TIME = UNFOLD_TIME * 1./1.;
	const double HintBox::SECS_PER_CHAR = 1./20.;

	HintBox::HintBox(const std::string& name, bool isTypewriter) : OverlayBox(name), FANLib::MCBSystem(_last), closed(true), closing(false), myQueue(0), delay(0),
		typewriter(isTypewriter), timeToNextChar(0), currentLength(0), animal(0)
	{
		setFinalSize(0.3, 0.8);		/// default
		setFinalFontHeight(0.045);	/// default
		setRelativePosition(0.93, 0.5);
	}

	HintBox::~HintBox() {
	}

	void HintBox::open(void* queue) {
		myQueue = (Queue*)queue;
		open();
	}

	void HintBox::setFinalSize(double x, double y) {
		mediumSize = Ogre::Vector2( screen.x * x, 0 ); /// leave as is, that's great without getPixelRatio!
		finalSize = Ogre::Vector2( screen.x * x, screen.y * y );
	}

	void HintBox::setFinalFontHeight(double y) {
		finalFontHeight = screen.y * y;
	}

	void HintBox::setRelativePosition(double x, double y) {
		setPosition(Ogre::Vector2( (x * screen.x - finalSize.x / 2.) * P3DWindow::getPixelRatio(), y * screen.y ));	/// default
	}

	void HintBox::setText(const std::string& str) {
		if (typewriter) {
			actualText = str;
			FANLib::String::replaceAll<Ogre::UTFString>(actualText, "\\n", "\n");
			this->OverlayBox::setText("");
			currentLength = 0;
		} else {
			this->OverlayBox::setText(str);
		}
	}

	void HintBox::open() {

		if (!closed) {
			return; /// <!> error <?>
		}
		closed = false;
		closing = false;

		/// non-changing properties
		setFontHeight( 0 );

		/// tweened properties
		setSize(Ogre::Vector2(0,0)); /// initial size : set AFTER border size
		setScale(Ogre::Vector2(0,0));

		deleteTweens(); /// just in case
		TPlaylist* list;

		// show
		ogreBox->show();
		shadowText->setVisible(true);
		editText->setVisible(true);

		/// list 1 : unfold
		list = new TPlaylist();
		/// - unfold x
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, mediumSize, APPEAR_TIME, &OverlayBox::getSize, &OverlayBox::setSize, delay)
			);
		/// - unfold y
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, finalSize, UNFOLD_TIME, &OverlayBox::getSize, &OverlayBox::setSize)
			);
		registerList(list);

		/// list 2 : text unfold
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, int>(
			this, finalFontHeight, UNFOLD_TEXT_TIME, &OverlayBox::getFontHeight, &OverlayBox::setFontHeight, APPEAR_TIME + UNFOLD_TIME - UNFOLD_TEXT_TIME + delay)
			);
		registerList(list);

		/// list 3 : appear
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2::UNIT_SCALE, APPEAR_TIME, &OverlayBox::getScale, &OverlayBox::setScale, delay)
			);
		registerList(list);

		// animal accompaniment
		if (animal) animal->talk(true);
	}

	void HintBox::close() {

		if (closing || closed) {
			return; /// <!> error <?>
		}
		closing = true;

		callBackSafe(CLOSING);

		/// hid
		acceptMouseEvents(false);
		Ogre::Vector2 fancyMediumSize = mediumSize;
		fancyMediumSize.x += screen.x * 0.1;

		deleteTweens(); /// just in case
		TPlaylist* list;

		/// list 1 : fold
		list = new TPlaylist();
		/// - fold y
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, fancyMediumSize, UNFOLD_TIME, &OverlayBox::getSize, &OverlayBox::setSize)
			);
		/// - fold x
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2(0,0), APPEAR_TIME, &OverlayBox::getSize, &OverlayBox::setSize)
			);
		registerList(list);

		/// list 2 : text fold
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, int>(
			this, 0, UNFOLD_TEXT_TIME, &OverlayBox::getFontHeight, &OverlayBox::setFontHeight)
			);
		registerList(list);

		/// list 3 : disappear
		list = new TPlaylist();
		list->add(new TLinear<OverlayBox, Ogre::Vector2>(
			this, Ogre::Vector2::ZERO, APPEAR_TIME, &OverlayBox::getScale, &OverlayBox::setScale, UNFOLD_TIME)
			);
		registerList(list);

		if (animal) animal->talk(false);
	}

	void HintBox::onMouseDown() {
		if (HIDBoss::getSingletonPtr()->isButtonDown(HIDBoss::MOUSE_LEFT)) close();
	}

	void HintBox::playlistFinished(TPlaylist* list) {

		this->TPlayed::playlistFinished(list);
		if (lists.size()) return; /// more lists pending, not finished yet!

		if (!closing) {

			acceptMouseEvents(true);
			if (typewriter)
			{
				VisualBoss::getSingletonPtr()->insert( this );
			}

		} else {

			closeFinished();
		}

	}

	bool HintBox::run(double time)
	{
		static const char skipHash = '#';

		timeToNextChar -= time;
		while (timeToNextChar <= 0 && currentLength < actualText.size()) {
			if ( actualText.at( currentLength ) == skipHash ) currentLength += 7;
			++currentLength;
			timeToNextChar += SECS_PER_CHAR;
		}
		this->OverlayBox::setText( actualText.substr( 0, currentLength ) );

		if ( currentLength >= actualText.size() )
			VisualBoss::getSingletonPtr()->remove( this );

		return false;
	}

	void HintBox::closeFinished() {
		if (typewriter) VisualBoss::getSingletonPtr()->remove( this );
		ogreBox->hide();
		shadowText->setVisible(false);
		editText->setVisible(false);

		closed = true;
		if (myQueue) {
			myQueue->next(); /// next!
			myQueue = 0;
		}
	}
}