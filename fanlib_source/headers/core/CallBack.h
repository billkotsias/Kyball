// Simple CallBack System
#pragma once
#ifndef FANLIB_CallBack_H
#define FANLIB_CallBack_H

namespace FANLib {

	//

	class ICallBack {
	public:

		virtual ~ICallBack() {};
		virtual void callBack() = 0;
	};

	//

	template <class Receiver>
	class CallBack : public ICallBack {

	protected:

		Receiver* receiver;
		void(Receiver::*function)(void*);
		void* sender;

	public:

		CallBack(Receiver* receiver, void(Receiver::*func)(void*), void* sender) : receiver(receiver), function(func), sender(sender) {
		};
		virtual ~CallBack() {};

		virtual void callBack() {
			(receiver->*function)(sender);
		};
	};

	//

	// Simple single-return callback system
	// See MCBSystem for a mutliple-callbacks implementation
	class CallBackSystem {

	private:

		ICallBack* iCallBack;

	protected:

		void callBack() {
			iCallBack->callBack();
		};
		void callBackSafe() {
			if (iCallBack) iCallBack->callBack();
		};

	public:

		CallBackSystem() : iCallBack(0) {};
		virtual ~CallBackSystem() { delete iCallBack; };

		template <class Receiver>
		void setCallBack(Receiver* receiver, void(Receiver::*function)(void*)) {
			delete iCallBack;
			iCallBack = new CallBack<Receiver>(receiver, function, this);
		};

		void unsetCallBack() {
			delete iCallBack;
			iCallBack = 0;
		};
	};

}

#endif