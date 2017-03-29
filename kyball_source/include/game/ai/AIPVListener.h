// ------------.................................
// AIPVListener - AIPVAnime listener
// ------------`````````````````````````````````

#pragma once
#ifndef AIPVListener_H
#define AIPVListener_H

namespace P3D {

	class AIPVAnime;

	class AIPVListener {

	public:

		virtual void animeOver(AIPVAnime*) = 0; /// this AIPVAnime you were listening to has just finished and will be deleted
	};

}

#endif