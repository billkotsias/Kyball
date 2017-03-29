// ---------------------
// GameQuality - user selects according to computer's power
// ---------------------

#pragma once
#ifndef GameQuality_H
#define GameQuality_H

namespace P3D {

	class GameQuality {

	public:

		enum Option {
			LOW,
			MEDIUM,
			HIGH,

			ADVANCED,
			SUPA_DUPA,

			NOT_USED_AFTERALL,
		};
	};

}

#endif
