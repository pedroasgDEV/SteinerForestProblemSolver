#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>

#include "Graph.hpp"

/**
 * @class DijkstraEngine
 * @brief Helper class designed to execute Dijkstra's algorithm repeatedly with
 * high performance.
 * * This class uses persistent memory and a token-based system (Lazy Reset)
 * to avoid expensive O(N) memory allocations and initializations on every call.
 * * Updated with Hop-bounded (Spatially Delimited) execution capabilities.
 */
class DijkstraEngine {
 private:
  const std::shared_ptr<Graph> graph;
  std::vector<float> dist;
  std::vector<int> hopsCount; 
  std::vector<unsigned long long int> visitedToken;
  std::vector<std::pair<int, int>> parent;  
  unsigned long long int currentToken;  

  using Pii = std::pair<float, int>;
  std::vector<Pii> pq;

 public:
  /**
   * @brief Constructor. Allocates memory once.
   * @param graph The reference graph
   */
  DijkstraEngine(const std::shared_ptr<Graph> graph) : graph(graph), currentToken(0){
    dist.resize(graph->nNodes);
    parent.resize(graph->nNodes);
    visitedToken.resize(graph->nNodes, 0);
    hopsCount.resize(graph->nNodes, 0); 
    pq.reserve(graph->nEdges);
  }

  /**
   * @brief Computes the shortest path between source and target.
   * @param source The starting node ID.
   * @param target The destination node ID.
   * @param bridges Optional bitmask of edges with 0 cost.
   * @param ditchs Optional bitmask of edges with infinite cost (ignored).
   * @param maxHops Geodesic expansion limit. -1 disables the limit (Default).
   * @return A pair containing the path and the cost.
   */
  std::pair<std::vector<int>, float> getShortPath(
      const int source, const int target,
      const std::vector<uint8_t>* bridges = nullptr,
      const std::vector<uint8_t>* ditchs = nullptr,
      const int maxHops = -1) { 
      
    currentToken++;  
    
    pq.clear();

    dist[source] = 0.0f;
    hopsCount[source] = 0;
    visitedToken[source] = currentToken; 
    parent[source] = {-1, -1};
    pq.push_back({0.0f, source});

    const auto& ptrs = graph->ptrs;
    const auto& edges = graph->edges;

    bool found = false;

    while (!pq.empty()) {
      std::pop_heap(pq.begin(), pq.end(), std::greater<Pii>());
      float d = pq.back().first;
      int u = pq.back().second;
      pq.pop_back();

      if (visitedToken[u] == currentToken && d > dist[u]) continue;

      if (u == target) {
        found = true;
        break;
      }

      // Ignore if the constraint is active (!= -1) and reach the radius limit, 
      if (maxHops != -1 && hopsCount[u] >= maxHops) continue; 

      for (int i = ptrs[u]; i < ptrs[u + 1]; ++i) {
        if (ditchs && (*ditchs)[i]) continue;

        const auto& edge = edges[i];
        float edgeCost = edge.weight;

        if (bridges && (*bridges)[i]) {
            edgeCost = 0.0f;
        }

        int v = edge.target;
        float newDist = d + edgeCost;

        bool isFirstVisit = (visitedToken[v] != currentToken);

        if (isFirstVisit || newDist < dist[v]) {
          dist[v] = newDist;
          parent[v] = {u, i};
          visitedToken[v] = currentToken;
          hopsCount[v] = hopsCount[u] + 1;
          pq.push_back({newDist, v});
          std::push_heap(pq.begin(), pq.end(), std::greater<Pii>());
        }
      }
    }

    if (!found) return {{}, -1.0f};

    std::vector<int> path;
    path.reserve(graph->nNodes / 10);  

    int curr = target;
    while (curr != source && curr != -1) {
      int node = parent[curr].first;
      int edge = parent[curr].second;

      if (node == -1) break;

      path.push_back(edge);
      curr = node;
    }

    return {path, dist[target]};
  }
};

#endif
