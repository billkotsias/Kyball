#include "StdAfx.h"
#include "tween\ITween.h"
#include "tween\TPlayer.h"

namespace P3D {

	// TPlayer
	TPlayer* TPlayer::instance = 0;	/// singleton instance not created yet

	TPlayer::TPlayer() {
	}

	TPlayer* TPlayer::getSingletonPtr() {
		if (!instance) instance = new TPlayer();
		return instance;
	}

	void TPlayer::destroy() {
		if (instance) delete instance;
		instance = 0;
	}

	TPlayer::~TPlayer() {
		instance = 0; /// putting this here allows someone to directly delete TPlayer*, bypassing the static 'destroy()' function (if friend class, that is)
		deleteAll();
	}

	void TPlayer::deleteAll() {
		for (std::map<TPlaylist*, bool>::iterator it = lists.begin(); it != lists.end(); ++it) {
			delete it->first;
		}
		lists.clear();
	}

	TPlaylist* TPlayer::addTween(P3D::ITween *tween) {
		TPlaylist* list = new TPlaylist();
		list->add(tween);
		addPlaylist(list);
		return list;
	}

	void TPlayer::run(double time) {
		for (std::map<TPlaylist*, bool>::iterator it = lists.begin(); it != lists.end(); ) {
			TPlaylist*const& list = it->first;
			if (list->run(time)) {
				delete list;
				lists.erase(it++);
			} else {
				++it;
			}
		}
	}

	void TPlayer::deletePlaylist(P3D::TPlaylist *list) {
		std::map<TPlaylist*, bool>::iterator it = lists.find(list);
		if (it == lists.end()) return;
		delete it->first;
		lists.erase(it);
	}

	// TPlaylist
	bool TPlaylist::run(double time) {
		while (time > 0) {

			ITween*& tween = tweens.front();
			time = tween->run(time);

			if (time >= 0) {							/// tween finished
				TweenListener* tweenListener = tween->getListener();
				if (tweenListener) tweenListener->tweenFinished(tween);
				delete tween;

				tweens.pop_front();
				if (tweens.size() == 0) {				/// playlist finished?
					if (listener) listener->playlistFinished(this);
					return true;
				}
				tweens.front()->start();				/// "start" next tween
			}
		}

		return false;	/// playlist not finished
	}

}