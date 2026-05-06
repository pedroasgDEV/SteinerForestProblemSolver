#ifndef BID_DIJKSTRA_HPP
#define BID_DIJKSTRA_HPP

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>

#include "Graph.hpp"

/**
 * @class BidirectionalDijkstraEngine
 */
class BidirectionalDijkstraEngine {
 private:
  const std::shared_ptr<Graph> graph;
  
  std::vector<float> distF;
  std::vector<int> hopsCountF; 
  std::vector<unsigned long long int> visitedTokenF;
  std::vector<std::pair<int, int>> parentF;  
  
  std::vector<float> distB;
  std::vector<int> hopsCountB; 
  std::vector<unsigned long long int> visitedTokenB;
  std::vector<std::pair<int, int>> parentB;  

  unsigned long long int currentToken;  

  using Pii = std::pair<float, int>;
  std::vector<Pii> pqF; 
  std::vector<Pii> pqB;

  inline float heuristic(int u, int v) const {
      return (u + v) * 0.0f; // only for remove the warnings
  }

 public:
  /**
   * @brief Constructor. Allocates frontier memory only once.
   * @param graph The reference graph
   */
  BidirectionalDijkstraEngine(const std::shared_ptr<Graph> graph) 
      : graph(graph), currentToken(0) {
    int n = graph->nNodes;
    
    distF.resize(n);            distB.resize(n);
    parentF.resize(n);          parentB.resize(n);
    visitedTokenF.resize(n, 0); visitedTokenB.resize(n, 0);
    hopsCountF.resize(n, 0);    hopsCountB.resize(n, 0);
    
    pqF.reserve(graph->nEdges / 2);
    pqB.reserve(graph->nEdges / 2);
  }

  /**
   * @brief Computes the shortest path bidirectionally between source and target.
   */
  std::pair<std::vector<int>, float> getShortPath(
      const int source, const int target,
      const std::vector<uint8_t>* bridges = nullptr,
      const std::vector<uint8_t>* ditchs = nullptr,
      const int maxHops = -1) { 
      
    if (source == target) return {{}, 0.0f};

    currentToken++;  
    pqF.clear(); pqB.clear();

    // Forward Initialization
    distF[source] = 0.0f;
    hopsCountF[source] = 0;
    visitedTokenF[source] = currentToken; 
    parentF[source] = {-1, -1};
    pqF.push_back({heuristic(source, target), source});

    // Backward Initialization
    distB[target] = 0.0f;
    hopsCountB[target] = 0;
    visitedTokenB[target] = currentToken; 
    parentB[target] = {-1, -1};
    pqB.push_back({heuristic(target, source), target});

    float bestPathCost = std::numeric_limits<float>::infinity();
    int meetingNode = -1;

    const auto& ptrs = graph->ptrs;
    const auto& edges = graph->edges;

    while (!pqF.empty() && !pqB.empty()) {
        
        float topF = pqF.front().first;
        float topB = pqB.front().first;
        if (topF + topB >= bestPathCost) {
            break; 
        }

        // Expand the leaner frontier (Guarantees a smaller explored area)
        if (pqF.size() <= pqB.size()) {
            std::pop_heap(pqF.begin(), pqF.end(), std::greater<Pii>());
            float f_u = pqF.back().first;
            int u = pqF.back().second;
            pqF.pop_back();

            // Lazy discard (if a better path was found before processing)
            if (distF[u] + heuristic(u, target) < f_u) continue;
            if (maxHops != -1 && hopsCountF[u] >= maxHops) continue; 

            for (int i = ptrs[u]; i < ptrs[u + 1]; ++i) {
                if (ditchs && (*ditchs)[i]) continue;

                const auto& edge = edges[i];
                int v = edge.target;
                float edgeCost = (bridges && (*bridges)[i]) ? 0.0f : edge.weight;
                float newDist = distF[u] + edgeCost;

                if (visitedTokenF[v] != currentToken || newDist < distF[v]) {
                    distF[v] = newDist;
                    parentF[v] = {u, i}; // {Previous Node, Edge ID}
                    visitedTokenF[v] = currentToken;
                    hopsCountF[v] = hopsCountF[u] + 1;
                    
                    pqF.push_back({newDist + heuristic(v, target), v});
                    std::push_heap(pqF.begin(), pqF.end(), std::greater<Pii>());

                    // Check for intersection with the Backward frontier
                    if (visitedTokenB[v] == currentToken) {
                        float pathCost = distF[v] + distB[v];
                        if (pathCost < bestPathCost) {
                            bestPathCost = pathCost;
                            meetingNode = v;
                        }
                    }
                }
            }
        } 
        else {
            // Backward Expansion (Reverse trajectory)
            std::pop_heap(pqB.begin(), pqB.end(), std::greater<Pii>());
            float f_u = pqB.back().first;
            int u = pqB.back().second;
            pqB.pop_back();

            if (distB[u] + heuristic(u, source) < f_u) continue;
            if (maxHops != -1 && hopsCountB[u] >= maxHops) continue; 

            for (int i = ptrs[u]; i < ptrs[u + 1]; ++i) {
                int rev_i = edges[i].reverseEdgePtr;
                if (rev_i == -1) continue; // Safety check for edges without return
                
                // In backward, check ditchs/bridges on the original edge (v -> u)
                if (ditchs && (*ditchs)[rev_i]) continue; 

                int v = edges[i].target;
                float edgeCost = (bridges && (*bridges)[rev_i]) ? 0.0f : edges[rev_i].weight;
                float newDist = distB[u] + edgeCost;

                if (visitedTokenB[v] != currentToken || newDist < distB[v]) {
                    distB[v] = newDist;
                    parentB[v] = {u, i}; // u explored v in backward via edge i (u->v)
                    visitedTokenB[v] = currentToken;
                    hopsCountB[v] = hopsCountB[u] + 1;
                    
                    pqB.push_back({newDist + heuristic(v, source), v});
                    std::push_heap(pqB.begin(), pqB.end(), std::greater<Pii>());

                    // Check for intersection with the Forward frontier
                    if (visitedTokenF[v] == currentToken) {
                        float pathCost = distF[v] + distB[v];
                        if (pathCost < bestPathCost) {
                            bestPathCost = pathCost;
                            meetingNode = v;
                        }
                    }
                }
            }
        }
    }

    // If the frontiers did not cross, there is no path
    if (meetingNode == -1) return {{}, -1.0f};

    std::vector<int> path;
    path.reserve(graph->nNodes / 10);  

    // Reconstruct the Forward part 
    int curr = meetingNode;
    std::vector<int> pathF;
    while (curr != source) {
        pathF.push_back(parentF[curr].second); 
        curr = parentF[curr].first;
    }
    std::reverse(pathF.begin(), pathF.end()); 

    // Reconstruct the Backward part 
    curr = meetingNode;
    std::vector<int> pathB;
    while (curr != target) {
        int edgeIndex = parentB[curr].second; 
        pathB.push_back(edges[edgeIndex].reverseEdgePtr); 
        curr = parentB[curr].first;
    }

    // Merge the two vectors
    path.insert(path.end(), pathF.begin(), pathF.end());
    path.insert(path.end(), pathB.begin(), pathB.end());

    return {path, bestPathCost};
  }
};

#endif
