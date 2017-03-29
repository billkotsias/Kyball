//////////////
// RuleManager
//////////////

#pragma once
#ifndef RuleManager_H
#define RuleManager_H

#include "Rule.h"
#include <vector>

namespace P3D {

	class Rule;

	class RuleManager {

	private:

		std::vector<Rule*> rules;

	public:

		~RuleManager();

		void addRule(Rule*);		/// add a rule; <we> are responsible for its <deletion>

		void initRules();			/// initialize all rules
		Rule::Result checkRules();	/// check all rules

		void clearRules();
	};
}

#endif