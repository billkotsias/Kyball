// An object that is enabled only if there is no object that has disabled it

#pragma once
#ifndef FANLIB_Enabled_H
#define FANLIB_Enabled_H

#include <map>

namespace FANLib {

	// DisableID (used by 'Enabled')

	class DisableID {

	private:

		static unsigned int uniqueID;

		unsigned int _id;
		DisableID(unsigned int __id) : _id(__id) {};

	public:

		/// copy constructor only
		DisableID(const DisableID& other) {
			_id = other.id();
		}

		inline unsigned int id() const { return _id; };

		inline bool operator<(const DisableID &other) const { return (_id < other.id()); };
		inline bool operator<=(const DisableID &other) const { return (_id <= other.id()); };
		inline bool operator==(const DisableID &other) const { return (_id == other.id()); };

		inline static DisableID get() { return DisableID(uniqueID++); };
	};

	// Enabled

	class Enabled {

	protected:

		bool f_Enabled;
		std::map<DisableID, bool> disablers;

	public:

		Enabled() : f_Enabled(true) {};

		virtual inline void disable(DisableID id) {
			disablers[id] = true;
			f_Enabled = false;
		};

		virtual inline void enable(DisableID id) {
			disablers.erase(id);
			if (disablers.empty()) f_Enabled = true;
		};

		inline bool f_IsEnabled() { return f_Enabled; };
	};
}

#endif