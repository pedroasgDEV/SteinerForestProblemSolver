#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <queue>
#include <utility>
#include <vector>

#include "../utils/Graph.hpp"

/**
 * @class DijkstraEngine
 * @brief Helper class designed to execute Dijkstra's algorithm repeatedly with
 * high performance.
 * * This class uses persistent memory and a token-based system (Lazy Reset)
 * to avoid expensive O(N) memory allocations and initializations on every call.
 */
class DijkstraEngine {
 private:
  std::vector<float> dist;
  std::vector<int> visitedToken;
  std::vector<std::pair<int, int>>
      parent;  // Stores the predecessor {node, edge} of each node for path
               // reconstruction.
  int nNodes;
  int currentToken;  // The "timestamp" or ID of the current Dijkstra execution.

  using Pii = std::pair<float, int>;
  std::priority_queue<Pii, std::vector<Pii>, std::greater<Pii>> pq;

 public:
  /**
   * @brief Constructor. Allocates memory once.
   * @param nodes Total number of nodes in the graph.
   */
  DijkstraEngine(int nodes) : nNodes(nodes), currentToken(0) {
    // Resize vectors once to prevent heap fragmentation during execution.
    dist.resize(nNodes);
    parent.resize(nNodes);
    visitedToken.resize(nNodes, 0);
  }

  /**
   * @brief Computes the shortest path between source and target.
   * @param graph The reference graph (CSR).
   * @param source The starting node ID.
   * @param target The destination node ID.
   * @return A pair containing:
   * 1. std::vector<int>: The sequence of edges (path). Empty if unreachable.
   * 2. float: The total cost of the path.
   */
  std::pair<std::vector<int>, float> getShortPath(const Graph& graph,
                                                  const int source,
                                                  const int target);
};

/**
 * @brief GRASP Constructive Heuristic.
 * @param originalGraph SFP initial graph.
 * @param terminals Pairs of terminals that need to be connected.
 * @param alpha RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
 * @return Graph with the SFP solution.
 */
Graph GRASPconstructiveHeuristic(
    const Graph& originalGraph,
    const std::vector<std::pair<int, int>>& terminals, float alpha = 1.0f);

#endif
