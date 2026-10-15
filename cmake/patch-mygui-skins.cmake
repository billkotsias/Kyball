# Applied from the MyGUI source root.
# MyGUI 3.2 renames the game's skin names (WordWrapSimple, StaticText, ...) to
# names this data set does not define, so menu captions are dropped. Keep the
# shipped skin when the renamed one was never loaded.
file(READ MyGUIEngine/src/MyGUI_BackwardCompatibility.cpp text)
if(NOT text MATCHES "KYBALL_SKIN_FALLBACK")
	string(REPLACE
"		if (item != mSkinRename.end())
		{
			MYGUI_LOG(Warning, (*item).first << \" skin is deprecated, use \" << (*item).second << \" [\" << LayoutManager::getInstance().getCurrentLayout() << \"]\");
			return (*item).second;
		}"
"		if (item != mSkinRename.end())
		{
			// KYBALL_SKIN_FALLBACK: these layouts still define the old skin.
			if (ResourceManager::getInstancePtr() != nullptr &&
				ResourceManager::getInstance().isExist((*item).second))
			{
				MYGUI_LOG(Warning, (*item).first << \" skin is deprecated, use \" << (*item).second << \" [\" << LayoutManager::getInstance().getCurrentLayout() << \"]\");
				return (*item).second;
			}
			return _skinName;
		}"
		text "${text}")
	file(WRITE MyGUIEngine/src/MyGUI_BackwardCompatibility.cpp "${text}")
endif()
