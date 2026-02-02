#include <iostream>
#include <vector>
#include <cassert>
#include <utility>

#include "../algorithms/algorithms.hpp" 
#include "../utils/graph.hpp"           
#include "tests.hpp"

// Helper to verify the path sequence
// Checks if the sequence of Node IDs matches expected.
// Also verifies if Edge IDs are consistent (-1 for start, >=0 for others).
static bool verifyPath(const std::vector<std::pair<int, int>>& resultPath, const std::vector<int>& expectedNodes) {
    if (resultPath.size() != expectedNodes.size()) {
        std::cerr << "Path size mismatch! Expected " << expectedNodes.size() << ", got " << resultPath.size() << std::endl;
        return false;
    }

    for (size_t i = 0; i < resultPath.size(); i++) {
        // Check Node ID
        if (resultPath[i].first != expectedNodes[i]) {
            std::cerr << "Node mismatch at index " << i << ". Expected " << expectedNodes[i] << ", got " << resultPath[i].first << std::endl;
            return false;
        }

        // Check Edge Index Consistency
        if (i == 0){
            // Start node must NOT have an incoming edge index (-1)
            if (resultPath[i].second != -1) {
                std::cerr << "Start node must have edge_index -1." << std::endl;
                return false;
            }
        }
        else {
            // Subsequent nodes MUST have a valid edge index (>= 0)
            if (resultPath[i].second < 0) {
                std::cerr << "Node " << resultPath[i].first << " has invalid edge index: " << resultPath[i].second << std::endl;
                return false;
            }
        }
    }
    return true;
}

void dijkstraTests() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "        STARTING DIJKSTRA TEST          " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "[Dijkstra] Simple path check... ";
    
    // Graph: 0-1 (10), 1-2 (10)
    Graph g1({ {0, 1, 10.0f}, {1, 2, 10.0f} }, 3, true);

    DijkstraResult res1 = dijkstra(g1, 0, 2);
    
    // Expect: 0 -> 1 -> 2
    assert(verifyPath(res1.path, {0, 1, 2}));
    assert(res1.cost == 20.0f);
    std::cout << "Passed." << std::endl;

    std::cout << "[Dijkstra] Shortcut check... ";
    
    // 0-1 (10), 1-2 (10), 0-2 (5)
    Graph g2({ 
        {0, 1, 10.0f}, 
        {1, 2, 10.0f}, 
        {0, 2, 5.0f} 
    }, 3, true);

    DijkstraResult res2 = dijkstra(g2, 0, 2);

    // Expect: 0 -> 2 (Direct path is cheaper)
    assert(verifyPath(res2.path, {0, 2}));
    assert(res2.cost == 5.0f);
    
    std::cout << "Passed." << std::endl;

    // ----------------------------------------------------------------
    std::cout << "[Dijkstra] Unreachable check... ";
    // Graph: Disconnected components {0, 1} and {2, 3}
    Graph g3({ 
        {0, 1, 5.0f}, 
        {2, 3, 5.0f} 
    }, 4, true);

    DijkstraResult res3 = dijkstra(g3, 0, 3);
    
    // Expect: Empty path
    assert(res3.path.empty());

    std::cout << "Passed." << std::endl;

    // ----------------------------------------------------------------
    std::cout << "[Dijkstra] Dynamic Obstacle (Soft Deletion)... ";
    // Graph: Two paths from 0 to 2
    // Path A: 0 -> 1 -> 2 (Cost 20)
    // Path B: 0 -> 3 -> 2 (Cost 100)
    Graph g4({
        {0, 1, 10.0f}, {1, 2, 10.0f}, // Cheap path
        {0, 3, 50.0f}, {3, 2, 50.0f}  // Expensive path
    }, 4, true);

    // Should take cheap path (0-1-2)
    DijkstraResult run1 = dijkstra(g4, 0, 2);
    assert(verifyPath(run1.path, {0, 1, 2}));
    assert(run1.cost == 20.0f);

    // Block the cheap path (remove edge 0-1)
    g4.inactiveEdge(0, 1);

    // Should take expensive path (0-3-2)
    DijkstraResult run2 = dijkstra(g4, 0, 2);
    assert(verifyPath(run2.path, {0, 3, 2}));
    assert(run2.cost == 100.0f);

    std::cout << "Passed." << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
    std::cout << "========================================" << std::endl;
}
