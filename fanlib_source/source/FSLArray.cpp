#include <fsl\FSLArray.h>
#include <fsl\FSLString.h>
#include <fsl\FSLArrayMessages.h>
#include <fsl\FSLPointer.h>
#include <fsl\FSLClass.h>

namespace FANLib {

	FSLArray::FSLArray() : type(FSL::NULL_TYPE) {
	}

	FSLArray::~FSLArray() {
#ifdef _DEBUG
		//Log::internalLog("FSLArray : deleting the remainings of.");
#endif
	}

	bool FSLArray::setType(FSL::VAR_TYPE inType) {
		if (type == FSL::NULL_TYPE) {
			type = inType;
			return false; /// ok
		}
		if (type == inType) return false; /// ok
		return true; /// error
	}

	void FSLArray::gc() {
		switch (type) {
			case FSL::ARRAY:
			case FSL::CLASS:
			case FSL::STRING:
				for (int i = data.size() - 1; i >= 0; --i) { /// delete 'data' in reverse order
					FSLObject* obj = *(FSLObject**)&(data.at(i));
					obj->decreasePointers(); /// no need to put 0 in data.at(i), except maybe for really-deep-debugging
				}
				break;
		}
	}

	//
	// get a variable value
	//
	inline void* FSLArray::get(const unsigned int& pos, const FSL::VAR_TYPE varType, const FastStr* errorStr) const {
#ifdef _DEBUG
/* error checking : debug mode only. If something breaks in release mode, remove preprocessor exclusion */;
		if (type != varType) {
			const FastStr* wrong_req_type[] = {errorStr, 0};
			Log::internalLog(WRONG_REQ_TYPE, wrong_req_type, true, FANLib::Error::FSL_CLASS_WRONG_REQ_TYPE);
		}
		if (pos >= data.size()) {
			Log::internalLog(&OUT_OF_BOUNDS, true, FANLib::Error::FSL_ARRAY_OUT_OF_BOUNDS);
		}
#endif
		return (void*)&data.at(pos);
	}

	int FSLArray::getInt(const unsigned int& pos) const {
		return *(int*)get(pos, FSL::INT, &WRONG_TYPE_INT);
	}

	FSL::real FSLArray::getReal(const unsigned int& pos) const {
		return *(FSL::real*)get(pos, FSL::REAL, &WRONG_TYPE_REAL);
	}

	char* FSLArray::getCString(const unsigned int& pos) const {
		return (*((FSLString**)get(pos, FSL::STRING, &WRONG_TYPE_STRING)))->getCString();
	}

	FSLClass* FSLArray::getClass(const unsigned int& pos) const {
		return (*(FSLClass**)get(pos, FSL::CLASS, &WRONG_TYPE_CLASS));
	}

	FSLArray* FSLArray::getArray(const unsigned int& pos) const {
		return (*(FSLArray**)get(pos, FSL::ARRAY, &WRONG_TYPE_ARRAY));
	}


	//
	// change a variable value
	//
	inline void FSLArray::change(const unsigned int& pos, const FSL::VAR_TYPE varType, const FastStr* errorStr) {
#ifdef _DEBUG
/* error checking : debug mode only. If something breaks in release mode, remove preprocessor exclusion */;
		if (type != varType) {
			const FastStr* wrong_req_type[] = {errorStr, 0};
			Log::internalLog(WRONG_DEST_TYPE, wrong_req_type, true, FANLib::Error::FSL_CLASS_WRONG_DEST_TYPE);
		}
		if (pos >= data.size()) {
			Log::internalLog(&OUT_OF_BOUNDS, true, FANLib::Error::FSL_ARRAY_OUT_OF_BOUNDS);
		}
#endif
	}

	void FSLArray::changeInt(const unsigned int &pos, int &value) {
		change(pos, FSL::INT, &WRONG_TYPE_INT);
		data.at(pos) = *(FSL::real*)&value;
	}

	void FSLArray::changeReal(const unsigned int &pos, FANLib::FSL::real &value) {
		change(pos, FSL::REAL, &WRONG_TYPE_REAL);
		data.at(pos) = *(FSL::real*)&value;
	}

	void FSLArray::changeString(const unsigned int &pos, FANLib::FSLString *value) {
		change(pos, FSL::STRING, &WRONG_TYPE_STRING);
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}

	void FSLArray::changeClass(const unsigned int &pos, FANLib::FSLClass *value) {
		change(pos, FSL::CLASS, &WRONG_TYPE_CLASS);

#ifdef _DEBUG
		/// old and new classes must have the same definition
		if (((FSLClass*)&data.at(pos))->definition != value->definition) {
			Log::internalLog(&WRONG_CLASS_DEF, true, FANLib::Error::FSL_CLASS_WRONG_DEST_TYPE);
		}
#endif
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}

	void FSLArray::changeArray(const unsigned int &pos, FANLib::FSLArray *value) {
		change(pos, FSL::ARRAY, &WRONG_TYPE_ARRAY);
		FSLPointer* ptr = (FSLPointer*)&data.at(pos); /// decrease/increase pointer-count respectively
		(*ptr) = value;
	}

}
