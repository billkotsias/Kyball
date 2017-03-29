#include "StdAfx.h"
#include "game\ai\AIPVAnimator.h"

#include "P3DApp.h"
#include "game\ai\AIPVAnime.h"
#include "game\ai\AIPVListener.h"

namespace P3D {

	AIPVAnimator::~AIPVAnimator() {
		for (std::map<AIPosVel*, std::map<AIPVAnime*, AIPVAParams> >::iterator it = animes.begin(); it != animes.end(); ++it) {
			std::map<AIPVAnime*, AIPVAParams>& objAnimes = it->second;
			for (std::map<AIPVAnime*, AIPVAParams>::iterator it2 = objAnimes.begin(); it2 != objAnimes.end(); ++it2) {
				delete it2->first;
			}
		}
	}

	void AIPVAnimator::newAnime(P3D::AIPVAnime *anime, double delay, AIPVListener* listener) {
		animes[anime->getObject()][anime] = AIPVAParams(delay * P3DApp::CYCLES_PER_SEC, listener);
	}

	void AIPVAnimator::deleteAnime(P3D::AIPVAnime *anime) {
		delete anime;

		std::map<AIPosVel*, std::map<AIPVAnime*, AIPVAParams> >::iterator it = animes.find(anime->getObject());
		if (it == animes.end()) return;

		std::map<AIPVAnime*, AIPVAParams>& objAnimes = it->second;
		objAnimes.erase(anime);
		if (objAnimes.size() == 0) animes.erase(it);
	}

	void AIPVAnimator::deleteAllAnimes(P3D::AIPosVel *ai) {
		std::map<AIPosVel*, std::map<AIPVAnime*, AIPVAParams> >::iterator it = animes.find(ai);
		if (it == animes.end()) return;

		/// - delete actual animes
		std::map<AIPVAnime*, AIPVAParams>& objAnimes = it->second;
		for (std::map<AIPVAnime*, AIPVAParams>::iterator it2 = objAnimes.begin(); it2 != objAnimes.end(); ++it2) {
			delete it2->first;
		}
		/// - remove references
		animes.erase(it);
	}

	void AIPVAnimator::run() {
		for (std::map<AIPosVel*, std::map<AIPVAnime*, AIPVAParams> >::iterator it = animes.begin(); it != animes.end(); ) { /// <!!!>
			std::map<AIPVAnime*, AIPVAParams>& objAnimes = it->second;

			for (std::map<AIPVAnime*, AIPVAParams>::iterator it2 = objAnimes.begin(); it2 != objAnimes.end(); ) { /// <!!!>

				/// - is there a start delay?
				AIPVAParams& params = it2->second;
				if (params.delay > 0) {
					--params.delay;
					++it2;
					continue;
				}

				/// - run Anime
				AIPVAnime* anime = it2->first;
				if (anime->run()) {
					AIPVListener* listener = params.listener;
					if (listener) listener->animeOver(anime);	/// notify listener this Anime has finished
					delete anime;			/// delete Anime
					objAnimes.erase(it2++);	/// remove reference
				} else {
					++it2;
				}
			}

			if (objAnimes.size() == 0) {
				animes.erase(it++);
			} else {
				++it;
			}
		}
	}
}