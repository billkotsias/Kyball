// Advanced CallBack System
#pragma once
#ifndef FANLIB_MCBSystem_H
#define FANLIB_MCBSystem_H

#include "CallBack.h"

#include <vector>

namespace FANLib {

	// Multiple-callbacks system
	class MCBSystem {

	private:

		std::vector<ICallBack*> iCallBacks;

	protected:

		void callBack(int i) {
			iCallBacks.at(i)->callBack();
		};
		void callBackSafe(int i) {
			ICallBack*& iCallBack = iCallBacks.at(i);
			if (iCallBack) iCallBack->callBack();
		};

	public:

		MCBSystem(int size) {
			iCallBacks.resize(size);
		};
		virtual ~MCBSystem() {
			for (int i = iCallBacks.size() - 1; i >= 0; --i) {
				delete iCallBacks.at(i);
			}
		};

		template <class Receiver>
		void setCallBack(int i, Receiver* receiver, void(Receiver::*function)(void*)) {
			delete iCallBacks.at(i);
			iCallBacks.at(i) = new CallBack<Receiver>(receiver, function, (MCBSystem*)(this)); /// <cast> to (MCBSystem*) when checking for equality !!!
		};
	};

}

#endif