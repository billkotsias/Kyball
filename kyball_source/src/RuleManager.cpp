#include "StdAfx.h"
#include "game\rule\RuleManager.h"
#include "game\rule\Rule.h"

namespace P3D {

	RuleManager::~RuleManager() {
		clearRules();
	}

	void RuleManager::addRule(Rule* rule) {
		rules.push_back(rule);
	}

	Rule::Result RuleManager::checkRules() {
		for (int i = rules.size() - 1; i >= 0; --i) {
			Rule::Result checkRes = rules[i]->perCycleCheck();
			if (checkRes != Rule::NOTHING) return checkRes;
		}
		return Rule::NOTHING;
	}

	void RuleManager::initRules() {
		for (int i = rules.size() - 1; i >= 0; --i) {
			rules[i]->init();
		}
	}

	void RuleManager::clearRules() {
		for (int i = rules.size() - 1; i >= 0; --i) {
			delete rules[i];
		}
		rules.clear();
	}
}