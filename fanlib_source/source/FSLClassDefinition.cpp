#include <fsl\FSLClassDefinition.h>
#include <fsl\FSLClassDefinitionMessages.h>

#ifdef _DEBUG
	#include <core\Utils.h>
#endif

namespace FANLib {

	FSLClassDefinition::FSLClassDefinition() : FSLClass(0) {
		definition = this; /// this is safer than to use 'FSLClass(this)' which produces a warning
	}

	FSLClassDefinition::~FSLClassDefinition() {
#ifdef _DEBUG
		//Log::internalLog("FSLClassDefinition : deleting the remainings of.");
#endif
	}

	FSLClass* FSLClassDefinition::createInstance() {
		FSLClass* fslClass = new FSLClass(this);			/// new class instance
		for (unsigned int i = 0; i < data.size(); ++i) {
			fslClass->data.push_back(data.at(i));			/// copy definition's default values to class instance (no <increase> in pointers -> when <finalized>)
		}
		return fslClass;
	}

	void FSLClassDefinition::finalize(FSLClass* fslClass) {
		for (unsigned int i = 0; i < data.size(); ++i) {
			/// check for <unique> values (<as yet unimplemented>)

			/// <increase> FSLObjects' pointers
			switch (getType(i)) {
				case FSL::ARRAY:
				case FSL::CLASS:
				case FSL::STRING:
					(*(FSLObject**)&(fslClass->data.at(i)))->increasePointers();
			}
		}
	}

	void FSLClassDefinition::gc() {
#ifdef _DEBUG
		Log::internalLog("GCing Class Definition with elements =", false);
		Log::internalLog(Utils::toString(data.size()).c_str(), true, 0, -1);
#endif
		gcClass();
		gcClassDef();
	}

	void FSLClassDefinition::gcClassDef() {
		/// delete all memory allocated for variable names
		/// - it would be nice to destroy strings in reverse order of their inserting, but <map> mixes their order; extra memory would be needed to achieve it
		for (std::map<FastStr, int>::iterator it = var.begin(); it != var.end(); ++it) {
#ifdef _DEBUG
				Log::internalLog("Deleting var name...", false);
				Log::internalLog(&it->first, true, 0, -1);
#endif
				delete [] it->first.str; /// because 'FastStr' destructor doesn't delete referenced string
		}
	}

	bool FSLClassDefinition::varExists(const FastStr &fstr) const {
		std::map<FastStr, int>::const_iterator it = var.find(fstr);
		return (!(it == var.end()));
	}

	int FSLClassDefinition::getPosition(const FastStr &fstr) const {
		std::map<FastStr, int>::const_iterator it = var.find(fstr);

		if (it == var.end()) {
			const FastStr* var_not_found[] = {&fstr, 0};
			Log::internalLog(VAR_NOT_FOUND, var_not_found, true, FANLib::Error::FSL_CLASS_VAR_NOT_FOUND);
		}
		return it->second;
	}

	unsigned int FSLClassDefinition::getFullType(const int &pos) const {
		return type.at(pos);
	}
	unsigned int FSLClassDefinition::getType(const int &pos) const {
		return type.at(pos) & FSL::TYPE_BITS;
	}

	bool FSLClassDefinition::defined() {
		return var.size() > 0;
	}

}