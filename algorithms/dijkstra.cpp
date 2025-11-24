#include <queue>
#include <limits>
#include <algorithm>
#include <utility>
#include <iostream>

#include "algorithms.hpp"

// Pair for Priority Queue: <Distance, NodeIndex>
using PII = std::pair<float, int>;

std::pair<std::vector<int>, float> dijkstra(const Graph& graph, int start_id, int end_id) {
    int n = graph.getNNode();
    
    if (start_id < 0 || start_id > n || end_id < 0 || end_id > n) {
        std::cerr << "[Algorithm Error] Invalid node IDs provided to Dijkstra." << std::endl;
        return {};
    }

    const auto matrix = graph.getMatrix(); 

    std::vector<float> dist(n, std::numeric_limits<float>::infinity());
    std::vector<int> parent(n, -1);
    
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

        for (int v = 0; v < n; v++) {
            if (matrix[u][v] != 0.0f) {
                float weight = matrix[u][v];

                // Relaxation Step
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    // If distance to end_id is still infinity, it's unreachable
    if (dist[end_id] == std::numeric_limits<float>::infinity()) {
        return {};
    }

    std::vector<int> path;
    // Backtrack from end_id to start_id
    for (int v = end_id; v != -1; v = parent[v]) {
        path.push_back(v); // Convert back to 1-based ID
    }
    
    // Reverse to get start_id -> end_id order
    std::reverse(path.begin(), path.end());

    return {path, dist[end_id]};
}