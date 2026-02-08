#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <queue>
#include <utility>
#include <vector>

#include "Graph.hpp"

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
                                                  const int target) {
    currentToken++;  // Increment the global token.

    pq = std::priority_queue<Pii, std::vector<Pii>, std::greater<Pii>>();
    dist[source] = 0.0f;
    visitedToken[source] = currentToken;  // Mark source as visited in THIS run
    parent[source] = {-1, -1};
    pq.push({0.0f, source});

    const auto& ptrs = graph.ptrs;
    const auto& edges = graph.edges;

    bool found = false;

    while (!pq.empty()) {
      float d = pq.top().first;
      int u = pq.top().second;
      pq.pop();

      // If the token is old, treat dist[u] as Infinity.
      // If token is current but we found a better path efficiently, skip.
      if (visitedToken[u] == currentToken && d > dist[u]) continue;

      if (u == target) {
        found = true;
        break;
      }

      for (int i = ptrs[u]; i < ptrs[u + 1]; ++i) {
        const auto& edge = edges[i];

        int v = edge.target;
        float newDist = d + edge.weight;

        // Lazy Initialization logic for neighbor 'v'
        bool isFirstVisit = (visitedToken[v] != currentToken);

        if (isFirstVisit || newDist < dist[v]) {
          dist[v] = newDist;
          parent[v] = {u, i};
          visitedToken[v] = currentToken;
          pq.push({newDist, v});
        }
      }
    }

    // Target unreachable
    if (!found) return {{}, -1.0f};

    std::vector<int> path;
    path.reserve(nNodes / 10);  // Reserve estimated size to avoid reallocations

    int curr = target;
    while (curr != source && curr != -1) {
      int node = parent[curr].first;
      int edge = parent[curr].second;

      if (node == -1) break;

      path.push_back(edge);
      curr = node;
    }

    // Return Inverted Path
    return {path, dist[target]};
  }
};

#endif
