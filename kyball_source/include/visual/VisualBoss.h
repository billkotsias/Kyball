////////////////////////
// A manager that runs its objects for an amount of time every frame.
// Its objects can also request this manager to delete them, thus they can be simply stored here (clean deletion)
////////////////////////

#pragma once
#ifndef VisualBoss_H
#define VisualBoss_H

#include <map>

namespace P3D {

	class Visual;

	class VisualBoss {

	private:

		// singleton logic - constructor
		VisualBoss();
		~VisualBoss();
		static VisualBoss* instance;

	public:

		// singleton logic
		static VisualBoss* getSingletonPtr();	/// create at 1st call
		static void destroy();					/// call destructor statically


	protected:

		std::map<Visual*, bool> visuals; /// "visuals" is not accurate; this in fact can be everything needed to run every frame for an amount of time

	public:

		// run all elements this amount of time
		void run(double);

		void insert(Visual* vis, bool unpaused = true);
		void remove(Visual*);

		void pause(Visual*);
		void unpause(Visual*);

		void deleteAll();

	};
}

#endif