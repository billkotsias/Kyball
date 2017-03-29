// Fantasia Scripting Language - Basic FSL Object

#pragma once
#ifndef FANLIB_FSLObject_H
#define FANLIB_FSLObject_H

namespace FANLib {

	class FSLObject {
	public://private

		int pointers;	/// number of FSLPointers pointing here
		virtual void increasePointers();
		virtual void decreasePointers();
		virtual void forceGC();	/// force pointers = 0 (should only be used at end of FSL program : will crash if there are active pointers to this object)
		virtual void gc() = 0;	/// perform pre-deletion maintenance : called by 'FSLRuntime'

	protected://protected

		virtual ~FSLObject(); /// !!!

	public:

		// codec
		FSLObject();

		friend class FSLPointer;
		friend class FSLRuntime;
		friend class FSLParser;
		friend class FSLClassDefPtr;
	};
}

#endif