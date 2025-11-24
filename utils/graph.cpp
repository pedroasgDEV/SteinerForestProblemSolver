#include "graph.hpp"

bool hasNegativeWeights(const Graph& g) {
    for (auto i : g.getMatrix())
        for (auto j : i) if (j < 0) return false;

    return true;
}

bool isGraphConnected(const Graph& g) {
    int n = g.getNNode();
    if (n == 0) return true;

    std::vector<bool> visited(n, false);
    std::queue<int> q;
    
    q.push(0);
    visited[0] = true;
    int count = 0;

    while(!q.empty()){
        int u = q.front();
        q.pop();
        count++;

        for(int v = 0; v < n; v++){
            if(g.isAdjacent(u, v) && !visited[v]){
                visited[v] = true;
                q.push(v);
            }
        }
    }
    
    return count == n;
}