#include "StdAfx.h"
#include "gui\OverlayBox.h"

#include "P3DWindow.h"
#include "SceneBaseCreator.h"
#include <OgreBorderPanelOverlayElement.h>

#include "math\Math.h"
#include "core\String.h"

namespace P3D {

	const double OverlayBox::MYGUI_BORDERPANEL_RATIO = 0.95;
	OverlayBox::BorderSizesMap OverlayBox::BorderSizes;

	OverlayBox::OverlayBox(const std::string& name, std::string clonePrefix) : capsOnly(false) {

		screen = P3DWindow::getSize();

		/// ogreBox
		Ogre::OverlayManager* overMan = Ogre::OverlayManager::getSingletonPtr();
		if ( clonePrefix.size() ) {
			cloned = true;
			ogreBoxCorners = static_cast<Ogre::BorderPanelOverlayElement*>( overMan->cloneOverlayElementFromTemplate(name, clonePrefix) );
			//std::cout << "ogreBoxCorners->getName" << ogreBoxCorners->getName() << "\n";
			clonePrefix += "/"; /// <!>
			ogreBoxEdgesTB = static_cast<Ogre::BorderPanelOverlayElement*>( ogreBoxCorners->getChild(clonePrefix + name + "/EdgesTB") );
			ogreBoxEdgesLR = static_cast<Ogre::BorderPanelOverlayElement*>( ogreBoxCorners->getChild(clonePrefix + name + "/EdgesLR") );
			ogreBox = overMan->create(clonePrefix + name);
			ogreBox->add2D( ogreBoxCorners );
			ogreBox->show();
		} else {
			cloned = false;
			ogreBox = overMan->getByName(name);
			ogreBoxCorners = (Ogre::BorderPanelOverlayElement*)ogreBox->getChild(name + "/Corners");
			ogreBoxEdgesTB = (Ogre::BorderPanelOverlayElement*)ogreBoxCorners->getChild(name + "/EdgesTB");
			ogreBoxEdgesLR = (Ogre::BorderPanelOverlayElement*)ogreBoxCorners->getChild(name + "/EdgesLR");
		}

		/// MyGUI
		widgets = MyGUI::LayoutManager::getInstance().loadLayout("OgreBoxText.xml", clonePrefix + name + "/");
		MyGUI::Gui* myGui = SceneBaseCreator::getSingletonPtr()->getMyGUI();

		shadowText = (MyGUI::Edit*)myGui->findWidgetT(clonePrefix + name + "/shadowText");
		shadowText->setTextAlign(MyGUI::Align::Center); /// MyGUI::Align::Left | MyGUI::Align::VCenter
		shadowText->setVisible(false);
		shadowText->setEnabled(false);

		editText = (MyGUI::Edit*)myGui->findWidgetT(clonePrefix + name + "/editText");
		editText->setTextAlign(MyGUI::Align::Center); /// MyGUI::Align::Left | MyGUI::Align::VCenter
		editText->setVisible(false);
		editText->setEnabled(false);

		/// init default values
		/// ogreBox
		scale = Ogre::Vector2(ogreBox->getScaleX(), ogreBox->getScaleY());
		position = Ogre::Vector2(ogreBox->getScrollX(), ogreBox->getScrollY());
		size = Ogre::Vector2(ogreBoxCorners->getWidth(), ogreBoxCorners->getHeight());

		const std::string& boxName = name + "/Corners"; /// was: ogreBoxCorners->getName();
		BorderSizesMap::iterator it = BorderSizes.lower_bound( boxName );
		if( it == BorderSizes.end() || (BorderSizes.key_comp()(boxName, it->first)) )
		{
			/// get scripted border size
			Ogre::Vector2 borderSize( ogreBoxCorners->getLeftBorderSize(), ogreBoxCorners->getRightBorderSize() ); /// only 2 values of script's "border_size" used
			borderSize *= (screen.y / 1000.);
			BorderSizes.insert( it, BorderSizesMap::value_type( boxName, borderSize ) );
			setBorderSize( borderSize );
			std::cout << "scripted:" << getBorderSize() << "\n";

		} else {

			/// reset previously read
			setBorderSize( it->second );
			std::cout << "prev set:" << getBorderSize() << "\n";
		}

		/// MyGUI
		fontHeight = editText->getFontHeight();
	}

	OverlayBox::~OverlayBox() {
		deleteTweens();
		MyGUI::LayoutManager::getInstance().unloadLayout(widgets);

		if (cloned) {
			Ogre::OverlayManager* overMan = Ogre::OverlayManager::getSingletonPtr();
			overMan->destroyOverlayElement( ogreBoxEdgesLR );
			overMan->destroyOverlayElement( ogreBoxEdgesTB );
			overMan->destroyOverlayElement( ogreBoxCorners );
			overMan->destroy(ogreBox);
		} else {
			ogreBox->hide(); /// just in case, so that it doesn't stay open and inert
		}
	}

	void OverlayBox::setText(const std::string& str) {
		/// replace "\n" with NL char
		Ogre::UTFString string = str;
		FANLib::String::replaceAll<Ogre::UTFString>(string, "\\n", "\n");
		if (capsOnly) FANLib::String::setCase(string, true);

		editText->setCaption(string.asUTF8());
		shadowText->setCaption( editText->getOnlyText() );
	}

	void OverlayBox::setFontHeight(const int& h) {
		fontHeight = h;
		editText->setFontHeight(scale.y * fontHeight);
		shadowText->setFontHeight(scale.y * fontHeight);
	}

	void OverlayBox::setScale(const Ogre::Vector2& sc) {
		scale = sc;

		/// ogreBox
		ogreBox->setScale(scale.x, scale.y);

		/// MyGUI
		setFontHeight(fontHeight); /// reset

		/// reset size
		setSize(size);
	}

	void OverlayBox::setPosition(const Ogre::Vector2& pos) {
		position = pos;

		/// ogreBox - what a hell!
		ogreBox->setScroll(
			(2*position.x/P3DWindow::getPixelRatio() - screen.x - ( (borderSize.x * 2 + size.x) * scale.x ) ) / screen.x,
			(-2*position.y + screen.y + ( (borderSize.y * 2 + size.y) * scale.y ) ) / screen.y
			);

		/// MyGUI
		editText->setPosition(
			FANLib::Math::round( position.x/P3DWindow::getPixelRatio() - (borderSize.x + size.x) / 2 * scale.x * MYGUI_BORDERPANEL_RATIO ),
			FANLib::Math::round( position.y - (borderSize.y + size.y) / 2 * scale.y * MYGUI_BORDERPANEL_RATIO )
			);
		shadowText->setPosition( editText->getPosition().left + 1, editText->getPosition().top + 1 );
	}

	void OverlayBox::setSize(const Ogre::Vector2& sz) {
		size = sz;

		/// ogreBox
		Ogre::Vector2 siz = size + borderSize * 2;
		ogreBoxCorners->setDimensions(siz.x, siz.y);
		ogreBoxEdgesLR->setDimensions(siz.x, siz.y);
		ogreBoxEdgesTB->setDimensions(siz.x, siz.y);

		/// MyGUI
		siz = (size + borderSize) * scale * MYGUI_BORDERPANEL_RATIO;
		siz.x = FANLib::Math::round( siz.x );
		siz.y = FANLib::Math::round( siz.y );
		editText->setSize( siz.x, siz.y );
		shadowText->setSize( siz.x, siz.y );

		/// reset position
		setPosition(position);
	}

	void OverlayBox::setBorderSize(const Ogre::Vector2& bor) {
		//std::cout << ogreBox->getName() << ":" << bor << "\n";
		borderSize = bor;

		/// ogreBox
		ogreBoxCorners->setBorderSize(borderSize.x, borderSize.y);
		ogreBoxEdgesLR->setBorderSize(borderSize.x, borderSize.y);
		ogreBoxEdgesTB->setBorderSize(borderSize.x, borderSize.y);

		/// reset size
		setSize(size);
	}
}