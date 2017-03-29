// ---------
// TPlayed - a "self-cleaning" object tweened through 'TPlayer'
// ---------

#pragma once
#ifndef TPlayed_H
#define TPlayed_H

#include "tween\TPlayer.h"

namespace P3D {

	class TPlayed : public TPlaylistListener {

	protected:

		std::set<TPlaylist*> lists; /// remove on destruction

	public:

		// "self-cleaning" - must be called prior to derived object's destruction!
		void deleteTweens() {
			TPlayer* player = TPlayer::getSingletonPtr();
			for (std::set<TPlaylist*>::iterator it = lists.begin(); it != lists.end(); ++it) {
				player->deletePlaylist(*it);
			}
			lists.clear(); /// !
		};

		// add tween/playlist in list of running playlists
		void registerList(TPlaylist* list) {
			TPlayer* player = TPlayer::getSingletonPtr();
			player->addPlaylist(list);
			list->setListener(this);
			lists.insert(list);
		};

		TPlaylist* registerTween(ITween* tween) {
			TPlayer* player = TPlayer::getSingletonPtr();
			TPlaylist* list = player->addTween(tween);
			list->setListener(this);
			lists.insert(list);
			return list;
		};

		// remove playlist from list of running-ones
		virtual void playlistFinished(TPlaylist* list) {
			lists.erase(list); /// remove finished 'Playlists' from list of running-ones
		};

		bool existsID(int i) {
			for (std::set<TPlaylist*>::iterator it = lists.begin(); it != lists.end(); ++it) {
				if ((*it)->id == i) return true;
			}
			return false;
		};

		// Returns true if deleted, false if not found
		bool deleteID(int i) {
			for (std::set<TPlaylist*>::iterator it = lists.begin(); it != lists.end(); ++it) {
				if ((*it)->id == i) {
					TPlayer::getSingletonPtr()->deletePlaylist(*it);
					lists.erase(*it);
					return true;
				}
			}
			return false;
		};
	};

}

#endif