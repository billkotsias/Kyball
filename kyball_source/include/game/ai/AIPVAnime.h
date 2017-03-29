// ------------.................................
// AIPVAnime - Interface
// ------------`````````````````````````````````

#pragma once
#ifndef AIPVAnime_H
#define AIPVAnime_H

namespace P3D {

	class AIPVAnimator;
	class AIPosVel;

	class AIPVAnime {

	protected:

		AIPosVel* obj;

	public:

		AIPVAnime(AIPosVel*);
		virtual ~AIPVAnime();

		inline AIPosVel* getObject() { return obj; };

		virtual bool run() = 0; /// run a cycle; <= true = finished
	};

}

#endif