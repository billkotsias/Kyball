////////////
// Visual element
////////////

#pragma once
#ifndef Visual_H
#define Visual_H

namespace P3D {

	class Visual {

	protected:

		virtual ~Visual();

		// => run this amount of time
		// <= true = delete me !!!
		/// NOTE FOR THE FUTURE : THIS TECHNIQUE IS GREAT IF YOU WANT TO HAVE SEVERAL DIFFERENT 'VisualBosses' (non-singleton) and don't want to have a pointer to
		/// manager in every single managed object.
		/// BUT THAT MEANS THAT YOU CAN'T DELETE'EM AT ANY TIME YOU LIKE! Only if you warn them 1 FRAME in advance and tell THEM tell THEIR MANAGER to kill them!

		virtual bool run(double) = 0;

		friend class VisualBoss;
		friend class GameplayCollection;
	};
}

#endif