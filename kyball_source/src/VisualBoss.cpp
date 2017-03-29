#include "StdAfx.h"
#include "visual\VisualBoss.h"

#include "visual\Visual.h"

namespace P3D {

	VisualBoss* VisualBoss::instance = 0;	/// singleton instance not created yet

	VisualBoss::VisualBoss() {
	}

	VisualBoss* VisualBoss::getSingletonPtr() {
		if (!instance) instance = new VisualBoss();
		return instance;
	}

	void VisualBoss::destroy() {
		if (instance) delete instance;
		instance = 0;
	}

	VisualBoss::~VisualBoss() {
		instance = 0; /// putting this here allows someone to directly delete VisualBoss*, bypassing the static 'destroy()' function (if friend class, that is)
		deleteAll();
	}

	void VisualBoss::deleteAll() {
		for (std::map<Visual*, bool>::iterator it = visuals.begin(); it != visuals.end(); ) {
			Visual* visual = it->first;
			++it;
			delete visual;
		}
		visuals.clear();
	}

	void VisualBoss::run(double time) {
		for (std::map<Visual*, bool>::iterator it = visuals.begin(); it != visuals.end(); ) { /// <!!!> no increment step
			Visual* visual = it->first;
			bool unpaused = it->second;
			++it;
			if (unpaused && visual->run(time)) {
				delete visual;
				/// visuals.erase(it++); /// this is automatically called in < ~Visual() >
			}
		}
	}

	void VisualBoss::insert(P3D::Visual *visual, bool unpaused) {
		visuals[visual] = unpaused;
	}

	void VisualBoss::remove(Visual* visual) {
		/// no <delete> here, object must request it in < run() > 
		visuals.erase(visual);
	}

	void VisualBoss::pause(P3D::Visual *vis) {
		std::map<Visual*, bool>::iterator it = visuals.find(vis);
		if (it != visuals.end()) it->second = false;
	}

	void VisualBoss::unpause(P3D::Visual *vis) {
		std::map<Visual*, bool>::iterator it = visuals.find(vis);
		if (it != visuals.end()) it->second = true;
	}
}