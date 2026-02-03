#ifndef SFP_HPP
#define SFP_HPP

#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "../utils/Graph.hpp"

// Forward declarations
class SFPProblem;
class SFPSolution;

enum class MoveType { ADD, REMOVE, SWAP };

/**
 * @struct SFPMove
 * @brief Represents a change to the solution.
 */
struct SFPMove {
  MoveType type;
  int edgeIndex;
  float costDelta;

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
 private:
  const SFPProblem* problem;     // Reference to context (Flyweight)
  std::vector<int> activeEdges;  // Bitmask
  float currentCost;             // Objective value cache

 public:
  SFPSolution(const SFPProblem& prob, bool startEmpty = true);

  // Getters and Helpers
  float getObjectiveValue() const { return currentCost; }
  bool isFeasible() const;
  bool isEdgeActive(int idx) const { return activeEdges[idx]; }
  const SFPProblem& getProblem() const { return *problem; }
  void applySolution();  // Apply the solution to the problem

  // --- Low Level Operations (Used by Moves) ---
  void addEdge(int edgeIdx);
  void removeEdge(int edgeIdx);

  // Overloads
  bool operator==(const SFPSolution& other);
  bool operator!=(const SFPSolution& other);
  bool operator>=(const SFPSolution& other);
  bool operator<=(const SFPSolution& other);
  bool operator>(const SFPSolution& other);
  bool operator<(const SFPSolution& other);

  friend std::iostream operator<<(std::ostream& out,
                                  const SFPSolution& solution);
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

  // Lazy initialization of neighborhoods (Flyweight parts)
  mutable std::shared_ptr<AddNeighbourhood> c_nbhood;
  mutable std::shared_ptr<RemoveNeighbourhood> l_nbhood;

 public:
  SFPProblem(const std::string& filename);
  explicit SFPProblem(std::shared_ptr<Graph> g,
                      const std::vector<std::pair<int, int>>& terminals)
      : graph(g), terminals(terminals), instanceName("Manual") {}

  // Factories
  SFPSolution empty_solution() const;
  SFPSolution random_solution() const;

  // Neighborhood Access
  std::shared_ptr<AddNeighbourhood> construction_neighbourhood() const;
  std::shared_ptr<RemoveNeighbourhood> local_neighbourhood() const;

  // Getters and Helpers
  const std::shared_ptr<Graph> getGraphPtr() const { return graph; }
  const std::vector<std::pair<int, int>>& getTerminals() const {
    return terminals;
  }
  int getNEdges() const { return graph->getNEdges(); }
  int getNNodes() const { return graph->getNNodes(); }
  std::string getName() const { return instanceName; }

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
