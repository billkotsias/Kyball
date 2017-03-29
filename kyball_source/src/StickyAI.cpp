#include "StdAfx.h"
#include "game\ai\StickyAI.h"
#include "game\action\StickAction.h"

#include "math\Math.h"

namespace P3D {

	StickyAI::StickyAI(GameplayObject* obj, Ogre::SceneNode* parent, const Ogre::Vector3& position, bool mover) : AIPosVel(obj, parent, position, mover) {
		mass = Infinite;
	}

	Action* StickyAI::getAction() {
		static StickAction stickAction(Action::HIGH);
		return &stickAction;
	}

	/// moved default 'link' behaviour to <creator>
	//bool StickyAI::attachForm(Form* inForm) {

	//	if (AIPosVel::attachForm(inForm)) return true; /// remove previously attached 'Form' & attach to 'pos' & attach 'vel'

	//	// - set 'FormState'
	//	//form->setState(myState);

	//	link(); /// only now can we <link> ! And we do <by default>, since this is a gameplay "structure"

	//	return false;
	//}

}