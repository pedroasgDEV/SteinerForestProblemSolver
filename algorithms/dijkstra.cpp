#include <queue>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "algorithms.hpp"

// Pair for Priority Queue: <Distance, NodeIndex>
using PII = std::pair<float, int>;

DijkstraResult dijkstra(const Graph& graph, const int start_id, const int end_id) {
    int n = graph.getNNode();
    
    if (start_id < 0 || start_id > n || end_id < 0 || end_id > n)
        throw std::runtime_error("Invalid node IDs provided to Dijkstra.");

    const auto& ptrs = graph.getPtrs();
    const auto& edges = graph.getEdges();

    std::vector<float> dist(n, std::numeric_limits<float>::infinity());
    std::vector<std::pair<int, int>> parent(n, {-1, -1});
    
    // std::greater causes the smallest element to appear at top()
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;

    dist[start_id] = 0.0f;
    pq.push({0.0f, start_id});

    while (!pq.empty()) {
        float d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (u == end_id) break;
        if (d > dist[u]) continue;

       for (int i = ptrs[u]; i < ptrs[u+1]; ++i) {
            const auto& edge = edges[i];

            if (!edge.active) continue;

            int v = edge.target;
            float weight = edge.weight;

            // Relaxation Step
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = {u, i};
                pq.push({dist[v], v});
            }
        }
    }

    // If distance to end_id is still infinity, it's unreachable
    if (dist[end_id] == std::numeric_limits<float>::infinity()) return {};

    DijkstraResult result;
    result.cost = dist[end_id];
    
    for (int node_id = end_id; node_id != -1; node_id = parent[node_id].first) {
        int edge_id = parent[node_id].second;
        result.path.push_back({node_id, edge_id}); // In start_id, edge_id is -1
    }
    
    std::reverse(result.path.begin(), result.path.end());

    return result;
}
