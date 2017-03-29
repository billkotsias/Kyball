#include "StdAfx.h"
#include "Queue.h"

namespace P3D {

	Queue::~Queue() {
		reset();
	}

	void Queue::next() {

		if (objects.empty()) return;

		FANLib::ICallBack* obj = objects.front();
		objects.pop_front();
		obj->callBack();
		delete obj;
	}

	void Queue::reset() {
		for (int i = objects.size() - 1; i >= 0; --i) {
			delete objects.at(i);
		}
		objects.clear();
	}

}