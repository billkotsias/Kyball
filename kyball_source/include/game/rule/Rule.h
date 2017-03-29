///////
// Rule - a gameplay rule; either per-cycle dependent or gameplay-event driven
///////

#pragma once
#ifndef Rule_H
#define Rule_H

namespace P3D {

	class GameplayBase;

	class Rule {

	protected:

		GameplayBase* base; /// keep base cached for every rule, so as not to pass it around every time it's called

	public:

		enum Result {
			NOTHING,
			WIN,
			LOSE,
			QUIT, /// hack, should be replaced with a "menu" value or something!
		};

		Rule(GameplayBase* inBase) : base(inBase) {};
		virtual ~Rule() {};
		virtual void init() {};

		virtual Rule::Result perCycleCheck() { return Rule::NOTHING; };
	};
}

#endif