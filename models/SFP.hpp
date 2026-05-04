#ifndef SFP_HPP
#define SFP_HPP

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../utils/Graph.hpp"

// Forward declarations
class SFPProblem;
struct SolutionPair;

/**
 * @struct SolutionEdge
 * @brief Represents an edge that is part of the solution.
 */
struct SolutionEdge {
  int id;
  int reverse_id;  ///< Reverse Edge index
  double weight;   ///< Edge weight
  mutable std::unordered_set<int> pairs;  ///< list of pairs that the edge connects (int::ptr -> SFPSolution.pairs)
  mutable double I_e; ///< Belly Index (Topological Inefficiency)

  SolutionEdge(const int id, const int reverse_id, const double weight)
      : id(id), reverse_id(reverse_id), weight(weight), I_e(0.0f) {
    if (reverse_id != -1 && id > reverse_id) {
      this->id = reverse_id;
      this->reverse_id = id;
    }
  }

  bool operator==(const SolutionEdge& other) const {
    return (id == other.id && reverse_id == other.reverse_id) ||
           (id == other.reverse_id && reverse_id == other.id);
  }
};

/**
 * @struct SolutionPair
 * @brief Represents a pair that is part of the solution.
 */
struct SolutionPair {
  int source;           ///< Edge index
  int target;           ///< Reverse Edge index
  double pathCost;      ///< Actual path cost
  double previousCost;  ///< Last path cost
  mutable std::unordered_set<int>
      edges;  ///< list of edges that connect the pair (int::ptr -> Graph.edges)
  mutable std::vector<int>
      competing_pairs_sum;  ///< list with the sum of intersections with other
                            ///< pairs

  SolutionPair(const int source, const int target, const int nTerms)
      : source(source),
        target(target),
        pathCost(0.0f),
        previousCost(0.0f),
        competing_pairs_sum(std::vector<int>(nTerms, 0)) {
    if (source > target) {
      this->source = target;
      this->target = source;
    }
  }

  bool operator==(const SolutionPair& other) const {
    return (source == other.source && target == other.target) ||
           (target == other.source && source == other.target);
  }
};

struct EdgeHash {
  inline std::size_t operator()(const SolutionEdge& e) const {
    return static_cast<std::size_t>((static_cast<uint64_t>(e.id) << 32) |
                                    static_cast<uint32_t>(e.reverse_id));
  }
};

/**
 * @class SFPSolution
 * @brief Mutable state of the SFP.
 */
class SFPSolution {
  friend class SFPProblem;

 protected:
  const SFPProblem* problem;
  mutable std::vector<uint8_t> bitmask;
  double currentCost;
  mutable std::unordered_set<SolutionEdge, EdgeHash> edges;
  mutable std::vector<SolutionPair> pairs;

 public:
  SFPSolution(const SFPProblem* problem, std::vector<SolutionPair> pairs = {});

  bool isEdgeActive(const int edge_id) const;
  std::vector<SolutionEdge> getEdges() const {
    return {edges.begin(), edges.end()};
  }
  int getNEdges() const { return edges.size(); }
  std::vector<int> getEdgePairs(const SolutionEdge& edge) const;

  std::vector<int> getPairEdges(const int pair_id) const;
  double getPairCost(const int pair_id) const;
  std::pair<int, int> getPairNodes(const int pair_id) const {
    return {pairs[pair_id].source, pairs[pair_id].target};
  }
  void snapshotPairCost(const int pair_id) const {
    if(pair_id >= 0 && pair_id < (int)pairs.size())
        pairs[pair_id].previousCost = pairs[pair_id].pathCost;
  }
  double getPairPreviousCost(const int pair_id) const {
    if(pair_id >= 0 && pair_id < (int)pairs.size())
        return pairs[pair_id].previousCost;
    return -1.0;
  }
  const SolutionPair& getPair(const int pair_id) const { return pairs[pair_id]; }

  int getNPairs() const { return pairs.size(); }
  std::vector<int> getCompetingPairs(const int pair_id) const {
    std::vector<int> temp;
    temp.reserve(16);
    for (int it = 0; it < (int)pairs.size(); it++)
      if (pairs[pair_id].competing_pairs_sum[it] > 0) temp.push_back(it);
    return temp;
  }
  int getIntersectionsSum(const int pair_a, const int pair_b) const {
    return pairs[pair_a].competing_pairs_sum[pair_b];
  }

  const std::vector<uint8_t>* getBitmask() const { return &bitmask; }
  double getCurrentCost() const { return currentCost; }
  const SFPProblem* getProblem() const { return problem; }
  bool isFeasible() const;

  int insert(const int edge_id, const int pair_id);
  int erase(const int edge_id, const int pair_id = -1);

  bool operator>(const SFPSolution& other) const;
  bool operator<(const SFPSolution& other) const;

  friend std::ostream& operator<<(std::ostream& out, const SFPSolution& sol);
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
  double maxEdgeWeight;

 public:
  explicit SFPProblem(std::shared_ptr<Graph> g,
                      const std::vector<std::pair<int, int>>& terminals);

  SFPProblem() : graph(nullptr), instanceName("Empty") {}

  // Factories
  SFPSolution empty_solution() const { return SFPSolution(this); }

  const std::shared_ptr<Graph> getGraphPtr() const { return graph; }
  double getMaxEdgeWeight() const { return maxEdgeWeight; }
  const std::vector<std::pair<int, int>>& getTerminals() const { return terminals; }
  int getNEdges() const { return graph ? graph->nEdges : 0; }
  int getNNodes() const { return graph ? graph->nNodes : 0; }
  int getNTerminals() const { return terminals.size(); }
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

enum class MoveType { CNCT_PAIR, DSCNCT_PAIR };

/**
 * @class SFPMove
 * @brief Abstract interface for the metaheuristic moves.
 */
struct SFPMove {
  SFPSolution* solution;
  MoveType type;
  int pair_id;
  std::vector<int> connectedEdges;

  SFPMove(SFPSolution* solution, const MoveType type, const int pair_id,
          std::vector<int> pathIndices)
      : solution(solution),
        type(type),
        pair_id(pair_id),
        connectedEdges(std::move(pathIndices)) {}

  SFPMove(SFPMove&&) noexcept = default;
  SFPMove& operator=(SFPMove&&) noexcept = default;

  void apply() {
    for (const auto& edge_id : connectedEdges)
      if (type == MoveType::CNCT_PAIR) {
        if (!solution->insert(edge_id, pair_id))
          throw std::runtime_error(
              "CONNECT_MOVE_ERROR: An error occurred while applying the "
              "movement.");
      } else {
        if (!solution->erase(edge_id, pair_id))
          throw std::runtime_error(
              "DISCONECT_MOVE_ERROR: An error occurred while applying the "
              "movement.");
      }
  }

  void undo() {
    for (auto it = connectedEdges.rbegin(); it != connectedEdges.rend(); ++it)
      if (type != MoveType::CNCT_PAIR) {
        if (!solution->insert(*it, pair_id))
          throw std::runtime_error(
              "CONNECT_MOVE_ERROR: An error occurred while applying the "
              "movement.");
      } else {
        if (!solution->erase(*it, pair_id))
          throw std::runtime_error(
              "DISCONECT_MOVE_ERROR: An error occurred while applying the "
              "movement.");
      }
  }
};

struct SFPNeighborhood {
  std::vector<SFPMove> moves;

  SFPNeighborhood() = default;

  void addMoveApplying(SFPMove move) {
    moves.push_back(std::move(move));
    moves.back().apply();
  }

  void apply() {
    for (auto& move : moves) move.apply();
  }

  void undo() {
    for (auto it = moves.rbegin(); it != moves.rend(); ++it) it->undo();
  }
};

#endif
