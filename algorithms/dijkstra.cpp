#include "algorithms.hpp"   

std::pair<std::vector<int>, float> DijkstraEngine::getShortPath(const Graph& graph, const int source, const int target){   
        currentToken++; // Increment the global token.

        pq = std::priority_queue<Pii, std::vector<Pii>, std::greater<Pii>>();
        dist[source] = 0.0f;
        visitedToken[source] = currentToken; // Mark source as visited in THIS run
        parent[source] = {-1, -1};
        pq.push({0.0f, source}) ;

        const auto& ptrs = graph.getPtrs();
        const auto& edges = graph.getEdges();

        bool found = false;

        while (!pq.empty()) {
            float d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            // If the token is old, treat dist[u] as Infinity.
            // If token is current but we found a better path efficiently, skip.
            if (visitedToken[u] == currentToken && d > dist[u]) continue;

            if (u == target) { found = true; break; }

            for (int i = ptrs[u]; i < ptrs[u + 1]; ++i) {
                const auto& edge = edges[i];

                if (!edge.active) continue;

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
        path.reserve(nNodes / 10); // Reserve estimated size to avoid reallocations (maybe happen, mas its will be lass than reserve all)                  

        int curr = target;
        while (curr != source && curr != -1) {
            int node = parent[curr].first;
            int edge = parent[curr].second;

            if(node == -1) break;

            path.push_back(edge);
            curr = node;
        }
        
        // Return Inverted Path
        return {path, dist[target]};  
}
