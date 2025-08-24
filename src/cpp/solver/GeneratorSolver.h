#pragma once
#include "./Op.h"
#include "minisat/core/Solver.h"

#include <set>
#include <memory>



struct Cell {
	size_t row;
	size_t col;

	bool operator==(const Cell& other) const {
		return row == other.row && col == other.col;
	}
	bool operator<(const Cell& other) const {
		if (row != other.row)
			return row < other.row;
		return col < other.col;
	}
};
struct NodePair {
	size_t pairIndex;
	char contents;
	Cell firstNode;
	Cell secondNode;

	bool operator==(const NodePair& other) const {
		return firstNode == other.firstNode && secondNode == other.secondNode;
	}
};

class GeneratorSolver {
private:
	size_t m_iRows;
	size_t m_iCols;
	std::vector<NodePair> m_pNodePairs;
	std::set<Cell> m_pNodes;
	std::vector<Variable> m_pVariables;
	std::unique_ptr<Minisat::Solver> m_minisatSolver;

	std::vector<std::vector<char>> m_solvedGrid;
public:
	GeneratorSolver(const std::vector<NodePair> &nodePairs, size_t rows, size_t cols);
	~GeneratorSolver();
	std::vector<Variable> initVariables() const;
	void initNodePairs(const std::vector<NodePair> &nodePairs);
	std::vector<Cell> getNeighbors(Cell cell) const;
	inline size_t getRows() const { return this->m_iRows; }
	inline size_t getCols() const { return this->m_iCols; }
	inline char getSolvedGridValue(size_t row, size_t col) const { return this->m_solvedGrid[row][col]; };
	inline std::vector<std::vector<char>> getSolvedGrid() const { return this->m_solvedGrid; };

	bool solve();
private:
	void addCellsAtLeastOneColorClauses();
	void addCellsAtMostOneColorClauses();
	void addMarkedNodeClauses();
	void addNodesOneNeighborClauses();
	void addNonNodeTwoNeighborClauses();

	void addClauseToSolver(Clause& clause);
	void addClausesToSolver(ClauseList& clauses);
};