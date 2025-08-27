#include "./GeneratorSolver.h"

GeneratorSolver::GeneratorSolver(const std::vector<NodePair> &nodePairs, size_t rows, size_t cols) {
	this->m_iRows = rows;
	this->m_iCols = cols;
	this->initNodePairs(nodePairs);
	this->m_minisatSolver = std::make_unique<Minisat::Solver>();
	this->m_pVariables = this->initVariables();
	for (size_t i = 0; i < this->m_pVariables.size(); i++)
		this->m_minisatSolver->newVar();

	for (size_t row = 0; row < this->m_iRows; row++) {
		std::vector<char> rowVector;
		for (size_t col = 0; col < this->m_iCols; col++)
			rowVector.push_back('.');
		this->m_solvedGrid.push_back(rowVector);
	}
}
GeneratorSolver::~GeneratorSolver() {
	
}
std::vector<Cell> GeneratorSolver::getNeighbors(Cell cell) const {
	const int cellDirections[4][2] = {
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
		{ 0, -1 }
	};
	std::vector<Cell> cellNeighbors;

	for (const int* cellDirection : cellDirections) {
		const long long neighborRow = (long long)cell.row + cellDirection[0];
		const long long neighborCol = (long long)cell.col + cellDirection[1];

		if (neighborRow >= 0 && neighborRow < (long long)this->m_iRows &&
			neighborCol >= 0 && neighborCol < (long long)this->m_iCols) {
			Cell cellNeighbor = { (size_t)neighborRow, (size_t)neighborCol };
			cellNeighbors.push_back(cellNeighbor);
		}
	}
	return cellNeighbors;
}
void GeneratorSolver::initNodePairs(const std::vector<NodePair> &nodePairs) {
	this->m_nodePairs = nodePairs;
	this->m_pNodes = {};
	for (NodePair nodePair : this->m_nodePairs) {
		this->m_pNodes.insert(nodePair.firstNode);
		this->m_pNodes.insert(nodePair.secondNode);
	}
	return;
}
std::vector<Variable> GeneratorSolver::initVariables() const {
	std::vector<Variable> variables;
	const size_t numPairs = this->m_nodePairs.size();

	for (size_t row_num = 0; row_num < this->m_iRows; row_num++) {
		for (size_t col_num = 0; col_num < this->m_iCols; col_num++) {
			for (size_t pairIndex = 0; pairIndex < numPairs; pairIndex++) {
				Variable variable = row_num * this->m_iCols * numPairs + col_num * numPairs + pairIndex + 1;
				variables.push_back(variable);
			}
		}
	}
	return variables;
}

void GeneratorSolver::addClauseToSolver(Clause& clause) {
	Minisat::vec<Minisat::Lit> minisatClause;

	for (Variable literal : clause) {
		Minisat::Var varIndex = static_cast<int>(std::abs(literal) - 1);
		bool isNegative = literal < 0;
		minisatClause.push(Minisat::mkLit(varIndex, isNegative));
	}
	this->m_minisatSolver->addClause(minisatClause);
}
void GeneratorSolver::addClausesToSolver(ClauseList& clauses) {
	for (Clause clause : clauses) {
		Minisat::vec<Minisat::Lit> minisatClause;

		for (Variable literal : clause) {
			Minisat::Var varIndex = static_cast<int>(std::abs(literal) - 1);
			bool isNegative = literal < 0;
			minisatClause.push(Minisat::mkLit(varIndex, isNegative));
		}
		this->m_minisatSolver->addClause(minisatClause);
	}
}

void GeneratorSolver::addCellsAtLeastOneColorClauses() {
	const size_t numPairs = this->m_nodePairs.size();

	for (size_t i = 0; i < this->m_pVariables.size(); i += numPairs) {
		std::vector<Variable> singleCellVarList = std::vector<Variable>(this->m_pVariables.begin() + i, this->m_pVariables.begin() + i + numPairs);
		Clause generatedClause = singleCellVarList;
		this->addClauseToSolver(generatedClause);
	}
}
void GeneratorSolver::addCellsAtMostOneColorClauses() {
	const size_t numPairs = this->m_nodePairs.size();

	for (size_t i = 0; i < this->m_pVariables.size(); i += numPairs) {
		std::vector<Variable> singleCellVarList = std::vector<Variable>(this->m_pVariables.begin() + i, this->m_pVariables.begin() + i + numPairs);
		ClauseList generatedClauses;
		Op::AtMostOne(singleCellVarList, generatedClauses);
		this->addClausesToSolver(generatedClauses);
	}
}
void GeneratorSolver::addMarkedNodeClauses() {
	const size_t numPairs = this->m_nodePairs.size();

	for (NodePair nodePair : this->m_nodePairs) {
		Cell firstNode = nodePair.firstNode;
		Cell secondNode = nodePair.secondNode;

		size_t firstVariableIndex = firstNode.row * this->m_iCols * numPairs + firstNode.col * numPairs + nodePair.pairIndex;
		size_t secondVariableIndex = secondNode.row * this->m_iCols * numPairs + secondNode.col * numPairs + nodePair.pairIndex;

		Variable firstVariable = this->m_pVariables[firstVariableIndex];
		Variable secondVariable = this->m_pVariables[secondVariableIndex];
		ClauseList generatedClauses = { { firstVariable }, { secondVariable } };
		this->addClausesToSolver(generatedClauses);
	}
}
void GeneratorSolver::addNodesOneNeighborClauses() {
	const size_t numPairs = this->m_nodePairs.size();

	for (NodePair nodePair : this->m_nodePairs) {
		ClauseList generatedClauses;
		Cell firstNode = nodePair.firstNode;
		Cell secondNode = nodePair.secondNode;
		
		std::vector<Cell> firstNodeNeighbors = this->getNeighbors(firstNode);
		std::vector<Variable> firstNeighborsVariables;
		for (Cell neighbor : firstNodeNeighbors) {
			size_t neighborVariableIndex = neighbor.row * this->m_iCols * numPairs + neighbor.col * numPairs + nodePair.pairIndex;
			firstNeighborsVariables.push_back(this->m_pVariables[neighborVariableIndex]);
		}

		

		std::vector<Cell> secondNodeNeighbors = this->getNeighbors(secondNode);
		std::vector<Variable> secondNeighborsVariables;
		for (Cell neighbor : secondNodeNeighbors) {
			size_t neighborVariableIndex = neighbor.row * this->m_iCols * numPairs + neighbor.col * numPairs + nodePair.pairIndex;
			secondNeighborsVariables.push_back(this->m_pVariables[neighborVariableIndex]);
		}
		Op::Unique(firstNeighborsVariables, generatedClauses);
		Op::Unique(secondNeighborsVariables, generatedClauses);
		this->addClausesToSolver(generatedClauses);
	}
}
void GeneratorSolver::addNonNodeTwoNeighborClauses() {
	const size_t numPairs = this->m_nodePairs.size();
	for (size_t row = 0; row < this->m_iRows; row++) {
		for (size_t col = 0; col < this->m_iCols; col++) {
			Cell currentCell = { row, col };
			if (this->m_pNodes.find(currentCell) != this->m_pNodes.end())
				continue;

			size_t currentCellVarIndexStart = row * this->m_iCols * numPairs + col * numPairs;
			std::vector<Cell> currentCellNeighbors = this->getNeighbors(currentCell);
			std::vector<size_t> neighborVarsIndexStart;
			for (Cell neighbor : currentCellNeighbors) {
				size_t neighborVarIndexStart = neighbor.row * this->m_iCols * numPairs + neighbor.col * numPairs;
				neighborVarsIndexStart.push_back(neighborVarIndexStart);
			}

			for (NodePair nodePair : this->m_nodePairs) {
				size_t currentCellVarIndex = currentCellVarIndexStart + nodePair.pairIndex;
				Variable currentCellVar = this->m_pVariables[currentCellVarIndex];

				Variable notCurrentCellVar = -currentCellVar;

				std::vector<Variable> neighborColorVars;
				for (size_t startIndex : neighborVarsIndexStart)
					neighborColorVars.push_back(this->m_pVariables[startIndex + nodePair.pairIndex]);

				ClauseList generatedClauses;
				Op::ExactlyTwo(neighborColorVars, generatedClauses);
				Op::Prepend(notCurrentCellVar, generatedClauses);

				this->addClausesToSolver(generatedClauses);
			}
		}
	}
}
bool GeneratorSolver::solve() {
	ClauseList allClauseList;
	const size_t numPairs = this->m_nodePairs.size();

	// this->addCellsAtLeastOneColorClauses, ONLY uncomment this if you want all cells to be filled.
	this->addCellsAtMostOneColorClauses();
	this->addMarkedNodeClauses();
	this->addNodesOneNeighborClauses();
	this->addNonNodeTwoNeighborClauses();

	bool satisfiable = this->m_minisatSolver->solve();
	if (!satisfiable)
		return false;
	
	for (size_t varIndex = 0; varIndex < this->m_minisatSolver->model.size(); varIndex++) {
		bool truthyVariable = this->m_minisatSolver->model[(int)varIndex].isTrue();
		if (!truthyVariable)
			continue;
		size_t pairIndex = varIndex % numPairs;
		size_t varCol = (varIndex / numPairs) % this->m_iCols;
		size_t varRow = (varIndex / numPairs) / this->m_iCols;
		for (NodePair nodePair : this->m_nodePairs) {
			if (nodePair.pairIndex == pairIndex)
				this->m_solvedGrid[varRow][varCol] = nodePair.contents;
		}
	}
	return true;
}
std::vector<CellPath> GeneratorSolver::getCellPaths() {
	std::vector<CellPath> cellPaths;
	
	if (!this->m_minisatSolver->okay())
		return {};
	
	for (NodePair nodePair : this->m_nodePairs) {
		CellPath currentCellPath;
		currentCellPath.nodePair = nodePair;
		currentCellPath.path.push_back(nodePair.firstNode);
		
		Cell lastCell = currentCellPath.path.back();

		while (lastCell != nodePair.secondNode) {
			bool pathExtended = false;
			std::vector<Cell> lastCellNeighbors = this->getNeighbors(lastCell);
			for (Cell lastCellNeighbor : lastCellNeighbors) {
				size_t neighborRow = lastCellNeighbor.row;
				size_t neighborCol = lastCellNeighbor.col;

				if (this->getSolvedGridValue(neighborRow, neighborCol) != currentCellPath.nodePair.contents)
					continue;
				if (currentCellPath.path.size() > 1) {
					Cell secondLastCell = *(currentCellPath.path.end() - 2);
					if (lastCellNeighbor == secondLastCell)
						continue;
				}
				lastCell = lastCellNeighbor;
				currentCellPath.path.push_back(lastCell);
				pathExtended = true;
				break;
			}
			if (!pathExtended)
				return {};
		}
		cellPaths.push_back(currentCellPath);
	}
	return cellPaths;
}