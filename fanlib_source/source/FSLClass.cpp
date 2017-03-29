#include <fsl\FSLClass.h>

#include <fsl\FSLTypes.h>
#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLString.h>
#include <fsl\FSLPointer.h>
#include <fsl\FSLArray.h>
#include <core\FastStr.h>
// error-handling
#include <fsl\FSLClassMessages.h>
#ifdef _DEBUG
	#include <core\Log.h>
	#include <FANLibErrors.h>
#endif

namespace FANLib {

	FSLClass::FSLClass(FSLClassDefinition* def) {
		definition = def;
	}

	FSLClass::~FSLClass() {
#ifdef _DEBUG
		//Log::internalLog("FSLClass : deleting the remainings of.");
#endif
	}

	void FSLClass::gc() {
		gcClass();
	}

	void FSLClass::gcClass() {
		/// 'FSLObject' children are not directly deleted but checked for Garbage Collection
		for (int i = data.size() - 1; i >= 0; --i) { /// delete 'data' in reverse order
			switch ((definition->getType(i))) {
				// was :
				// in all cases below, in 'data' there is an 'FSLPointer', which will tell the 'FSLObject' to decrease its pointers
				//FSLPointer* ptr;
				//case FSL::ARRAY:
				//case FSL::CLASS:
				//	Log::internalLog("Removing a pointer from a class");
				//	ptr = (FSLPointer*)&(data.at(i)); /// "hack" : 'FSLObject*' --> 'FSLPointer'
				//	(*ptr) = 0; /// null
				//	break;
				//case FSL::STRING:
				//	Log::internalLog("Removing a pointer from a string");
				//	ptr = (FSLPointer*)&(data.at(i)); /// "hack" : 'FSLObject*' --> 'FSLPointer'
				//	(*ptr) = 0; /// null
				//	break;

				/// in the cases below, 'data' contains an FSLObject*
				FSLObject* obj;
				case FSL::ARRAY:
				case FSL::CLASS:
				case FSL::STRING:
					obj = *(FSLObject**)&(data.at(i));
					obj->decreasePointers(); /// no need to put 0 in data.at(i), except maybe for really-deep-debugging
					break;
			}
		}

		/// ANOTHER NOTE : when definition gets destroyed, it does just that : definition = 0. But it gets destroyed AFTER
		///				   the GC cycle has finished. The definition will be GCed in the next GC cycle and we don't
		///				   want this. So, we nullify it here, right in this GC cycle.
		if (definition != this) definition = (FSLClassDefinition*)0; /// make null!
	}

	//
	// get a variable value
	//
	inline void* FSLClass::get(const FastStr& fstr, const FSL::VAR_TYPE varType, const FastStr* errorStr) const {
		int pos = definition->getPosition(fstr);
#ifdef _DEBUG
/* error checking : debug mode only. If something breaks in release mode, remove preprocessor exclusion */;
		if (definition->getType(pos) != varType) {
			const FastStr* wrong_req_type[] = {&fstr, errorStr, 0};
			Log::internalLog(WRONG_REQ_TYPE, wrong_req_type, true, FANLib::Error::FSL_CLASS_WRONG_REQ_TYPE);
		}
#endif
		return (void*)&data.at(pos);
	}

	int FSLClass::getInt(const FastStr& fstr) const {
		return *(int*)get(fstr, FSL::INT, &WRONG_TYPE_INT);
	}

	FSL::real FSLClass::getReal(const FastStr& fstr) const {
		return *(FSL::real*)get(fstr, FSL::REAL, &WRONG_TYPE_REAL);
	}

	char* FSLClass::getCString(const FastStr& fstr) const {
		return (*((FSLString**)get(fstr, FSL::STRING, &WRONG_TYPE_STRING)))->getCString();
	}

	FSLClass* FSLClass::getClass(const FastStr& fstr) const {
		return *((FSLClass**)get(fstr, FSL::CLASS, &WRONG_TYPE_CLASS));
	}

	FSLArray* FSLClass::getArray(const FastStr& fstr) const {
		return *((FSLArray**)get(fstr, FSL::ARRAY, &WRONG_TYPE_ARRAY));
	}


	//
	// change a variable value
	//
	inline int FSLClass::change(const FastStr& fstr, const FSL::VAR_TYPE varType, const FastStr* errorStr) {
		int pos = definition->getPosition(fstr);
#ifdef _DEBUG
/* error checking : debug mode only. If something breaks in release mode, remove preprocessor exclusion */;
		if (definition->getFullType(pos) != varType) {
			const FastStr* wrong_dest_type[] = {&fstr, errorStr, 0};
			Log::internalLog(WRONG_DEST_TYPE, wrong_dest_type, true, FANLib::Error::FSL_CLASS_WRONG_DEST_TYPE); /// side effect : 'unique' values can't change by mistake
		}
#endif
		return pos;
	}

	void FSLClass::changeInt(const FANLib::FastStr &fstr, int &value) {
		int pos = change(fstr, FSL::INT, &WRONG_TYPE_INT);
		data.at(pos) = *(FSL::real*)&value;
	}

	void FSLClass::changeReal(const FANLib::FastStr &fstr, FANLib::FSL::real &value) {
		int pos = change(fstr, FSL::REAL, &WRONG_TYPE_REAL);
		data.at(pos) = *(FSL::real*)&value;
	}

	void FSLClass::changeString(const FANLib::FastStr &fstr, FANLib::FSLString *value) {
		int pos = change(fstr, FSL::STRING, &WRONG_TYPE_STRING);
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}

	void FSLClass::changeClass(const FANLib::FastStr &fstr, FANLib::FSLClass *value) {
		int pos = change(fstr, FSL::CLASS, &WRONG_TYPE_CLASS);

#ifdef _DEBUG
		/// old and new classes must have the same definition
		if (((FSLClass*)&data.at(pos))->definition != value->definition) {
			const FastStr* wrong_class_def[] = {&fstr, 0};
			Log::internalLog(WRONG_CLASS_DEF, wrong_class_def, true, FANLib::Error::FSL_CLASS_WRONG_DEST_TYPE);
		}
#endif
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}

	void FSLClass::changeArray(const FANLib::FastStr &fstr, FANLib::FSLArray *value) {
		int pos = change(fstr, FSL::ARRAY, &WRONG_TYPE_ARRAY);
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}
}
