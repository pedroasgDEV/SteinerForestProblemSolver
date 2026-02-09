#ifndef SFP_HPP
#define SFP_HPP

#include <iostream>
#include <istream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <random>

#include "../utils/DSU.hpp"
#include "../utils/Dijkstra.hpp"
#include "../utils/Graph.hpp"

// Forward declarations
class SFPProblem;
class SFPSolution;

enum class MoveType { ADD, REMOVE };

/**
 * @struct SFPMove
 * @brief Represents a change to the solution.
 */
struct SFPMove {
  MoveType type;
  int edgeIndex;    ///< Index in the graph edges vector
  float costDelta;  ///< Pre-calculated cost of the move

  SFPMove(MoveType t, int idx, float delta)
      : type(t), edgeIndex(idx), costDelta(delta) {}

  void apply(SFPSolution& sol) const;
  void undo(SFPSolution& sol) const;
};

/**
 * @class SFPSolution
 * @brief Mutable state of the SFP.
 */
class SFPSolution {
  friend class SFPProblem;
  friend struct SFPMove;

 private:
  const SFPProblem* problem;      // Reference to context (Flyweight)
  std::vector<bool> activeEdges;  // Bitmask
  float currentCost;              // Objective value cache

  // Internal helpers (updates bitmask and reverse edge sync)
  void internalAdd(int edgeIdx);
  void internalRemove(int edgeIdx);

 public:
  explicit SFPSolution(const SFPProblem& problem);

  // Getters and Helpers
  float getObjectiveValue() const { return currentCost; }
  const SFPProblem& getProblem() const { return *problem; }
  bool isFeasible(
      DSU& dsu) const;  ///< Validates connectivity using an external DSU.
  bool isEdgeActive(int idx) const {
    return activeEdges[idx];
  }  ///< Verify if the edge are present in the solution.

  // Overloads
  bool operator>(const SFPSolution& other) const;
  bool operator<(const SFPSolution& other) const;

  friend std::ostream& operator<<(std::ostream& out, const SFPSolution& sol);
};

/**
 * @class SFPNeighborhood (Strategy / Factory)
 * @brief Abstract generator of moves.
 */
class SFPNeighborhood {
 protected:
  const SFPProblem& problem;

 public:
  explicit SFPNeighborhood(const SFPProblem& p) : problem(p) {}
  virtual ~SFPNeighborhood() = default;
  virtual std::vector<SFPMove> moves(const SFPSolution& sol) = 0;
};

// Concretes Neighborhoods
class AddNeighbourhood : public SFPNeighborhood {
 public:
  using SFPNeighborhood::SFPNeighborhood;
  std::vector<SFPMove> moves(const SFPSolution& sol) override;
};

class RemoveNeighbourhood : public SFPNeighborhood {
 public:
  using SFPNeighborhood::SFPNeighborhood;
  std::vector<SFPMove> moves(const SFPSolution& sol) override;
};

/**
 * @class SFPProblem
 * @brief Represents the static definition of a Steiner Forest Problem instance.
 * Contains the original graph and the terminal pairs. This object should be
 * read-only.
 */
class SFPProblem {
 private:
  std::shared_ptr<Graph> graph;
  std::vector<std::pair<int, int>> terminals;
  std::string instanceName;

 public:
  explicit SFPProblem(std::shared_ptr<Graph> g,
                      const std::vector<std::pair<int, int>>& terminals);

  SFPProblem() : graph(nullptr), instanceName("Empty") {}

  // Factories
  SFPSolution empty_solution() const;
  SFPSolution random_solution() const;

  // Getters and Helpers
  const std::shared_ptr<Graph> getGraphPtr() const { return graph; }
  const std::vector<std::pair<int, int>>& getTerminals() const {
    return terminals;
  }
  int getNEdges() const { return graph ? graph->nEdges : 0; }
  int getNNodes() const { return graph ? graph->nNodes : 0; }
  std::string getName() const { return instanceName; }
  void setName(const std::string name) { instanceName = name; }

  // Overloads
  /**
   * @brief Overload of the input operator to parse the specific file format.
   * * Format expected:
   * SECTION Graph
   * Nodes <Number of Nodes: Int>
   * Edges <Number of Edges: Int>
   * E <u from Nodes: int> <v from Nodes: int> <Edge Weight: int>
   * ...
   * END
   *
   * SECTION Terminals
   * Terminals <Number of terminals: Int>
   * TP <u from Nodes: int> <v from Nodes: int>
   * ...
   * END
   * @param in Input stream.
   * @param sf The SteinerForest object to populate.
   * @return std::istream& The input stream.
   */
  friend std::istream& operator>>(std::istream& in, SFPProblem& sf);

  /**
   * @brief Overloads the << operator to print the solution.
   * @param out The output stream.
   * @param sf The SteinerForest object.
   * @return The output stream.
   */
  friend std::ostream& operator<<(std::ostream& out, const SFPProblem& sf);
};

#endif
