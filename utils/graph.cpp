#include "graph.hpp"

bool hasNegativeWeights(const Graph& g) {
    for (auto i : g.getEdges()) if (i.weight < 0) return true;
    return false;
}

bool isGraphConnected(const Graph& g) {
    int nNode = g.getNNode();
    const auto& ptr = g.getPtrs();
    const auto& edges = g.getEdges();
    
    if (nNode == 0) return true;

    int start_node = 0;
    int count_visited = 0;
    
    std::vector<bool> visited(g.getNNode(), false);
    std::queue<int> q;

    visited[start_node] = true;
    q.push(start_node);
    count_visited++;

    while(!q.empty()){
        int u = q.front();
        q.pop();

        for(int i = ptr[u]; i < ptr[u+1]; ++i){
            if(edges[i].active && !visited[edges[i].target]){
                visited[edges[i].target] = true;
                count_visited++;
                q.push(edges[i].target);
            }
        }
    }

    return count_visited == g.getNNode();
}
