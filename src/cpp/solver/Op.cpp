#include "./Op.h"

Clause Op::Impl(Variable var1, Variable var2) {
	return { -var1, var2 };
}

void Op::Eq(Variable var1, Variable var2, ClauseList &outClauseList) {
	outClauseList.push_back(Op::Impl(var1, var2));
	outClauseList.push_back(Op::Impl(var2, var1));
}
void Op::Xor(Variable var1, Variable var2, ClauseList &outClauseList) {
	Op::Eq(var1, -var2, outClauseList);
}
void Op::Unique(std::vector<Variable> &varList, ClauseList &outClauseList) {
	Clause atLeastOneClause = (Clause)varList;
	outClauseList.push_back(atLeastOneClause);
	Op::AtMostOne(varList, outClauseList);
}
void Op::AtMostOne(std::vector<Variable>& varList, ClauseList& outClauseList) {
	size_t numVars = varList.size();
	for (size_t varIndex = 0; varIndex < numVars - 1; varIndex++) {
		Variable currVar = varList[varIndex];

		for (size_t nextVarIndex = varIndex + 1; nextVarIndex < numVars; nextVarIndex++) {
			Variable nextVar = varList[nextVarIndex];
			Clause clause = { -currVar, -nextVar };
			outClauseList.push_back(clause);
		}
	}
}
void Op::ExactlyTwo(std::vector<Variable> &varList, ClauseList &outClauseList) {
	size_t num_vars = varList.size();
	ClauseList clauses;
	switch (num_vars) {
	case 2: {
		Variable var1 = varList[0];
		Variable var2 = varList[1];
		outClauseList.push_back({ var1 });
		outClauseList.push_back({ var2 });
		break;
	}
	case 3: {
		Variable var1 = varList[0];
		Variable var2 = varList[1];
		Variable var3 = varList[2];

		outClauseList.push_back({ var1, var2 });
		outClauseList.push_back({ var1, var3 });
		outClauseList.push_back({ var2, var3 });
		outClauseList.push_back({ -var1, -var2, -var3 });
		
		break;
	}
	case 4: {
		Variable var1 = varList[0];
		Variable var2 = varList[1];
		Variable var3 = varList[2];
		Variable var4 = varList[3];

		outClauseList.push_back({ var1, var2, var3 });
		outClauseList.push_back({ var1, var2, var4 });
		outClauseList.push_back({ var1, var3, var4 });
		outClauseList.push_back({ var2, var3, var4 });
		outClauseList.push_back({ -var1, -var2, -var3 });
		outClauseList.push_back({ -var1, -var2, -var4 });
		outClauseList.push_back({ -var1, -var3, -var4 });
		outClauseList.push_back({ -var2, -var3, -var4 });
		
		break;
	}
	default: {
		// Unusual variable count detected.

		return;
	}
	}
}

void Op::Prepend(Variable prefixVar, ClauseList &clauses) {
	for (Clause &clause : clauses)
		clause.push_back(prefixVar);
}