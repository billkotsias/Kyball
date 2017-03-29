// ---------
// TPlayer - a basic "tween" manager
// ---------

#pragma once
#ifndef TPlayer_H
#define TPlayer_H

#include "ITween.h"

#include <deque>

namespace P3D {

	class TPlaylist;

	// TPlaylistListener
	class TPlaylistListener {
	public:
		virtual void playlistFinished(TPlaylist*) = 0;
	};


	// TPlaylist
	class TPlaylist {

	private:

		std::deque<ITween*> tweens;
		TPlaylistListener* listener;

		void start() { tweens.front()->start(); };

	public:

		TPlaylist() : listener(0), id(0) {};

		~TPlaylist() {
			for (unsigned int i = 0; i < tweens.size(); ++i) {
				delete tweens.at(i); /// delete <sequentially> so that tweened object gets <final condition>
			}
		};

		int id; /// set an ID to distinguish this list from others; <default> = 0

		inline TPlaylist* add(ITween* tween) { tweens.push_back(tween); return this; };
		inline void setListener(TPlaylistListener* listen) { listener = listen; };
		inline TPlaylistListener* getListener() { return listener; };

		bool run(double);

		friend class TPlayer;
	};


	// TPlayer
	class TPlayer {

	private:

		std::map<TPlaylist*, bool> lists; /// sequential-tween-playlist container

		// singleton logic - constructor
		TPlayer();
		~TPlayer();
		static TPlayer* instance;

	public:

		// singleton logic
		static TPlayer* getSingletonPtr();	/// create at 1st call
		static void destroy();				/// call destructor statically

		void addPlaylist(TPlaylist* list) {
			lists[list] = true;
			list->start();
		};
		TPlaylist* addTween(ITween*); /// for convenience to the user

		void deletePlaylist(TPlaylist*);

		void run(double);	/// run this amount of time

		void deleteAll();
	};

}
#endif