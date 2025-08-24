#pragma once
#include <vector>


typedef int Variable;
typedef std::vector<Variable> Clause;
typedef std::vector<Clause> ClauseList;

class Op {
public:
	static Clause Impl(Variable var1, Variable var2);

	static void Eq(Variable var1, Variable var2, ClauseList &outClauseList);
	static void Xor(Variable var1, Variable var2, ClauseList &outClauseList);
	static void Unique(std::vector<Variable> &varList, ClauseList &outClauseList);
	static void AtMostOne(std::vector<Variable> &varList, ClauseList &outClauseList);
	static void ExactlyTwo(std::vector<Variable> &varList, ClauseList &outClauseList);

	static void Prepend(Variable prefixVar, ClauseList &clauses);
};