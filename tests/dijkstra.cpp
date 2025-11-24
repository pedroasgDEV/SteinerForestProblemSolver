#include <iostream>
#include <vector>
#include <cassert>
#include <utility>

#include "../algorithms/algorithms.hpp" 
#include "../utils/graph.hpp"           
#include "tests.hpp"

static bool verifyPath(const std::vector<int>& actual, const std::vector<int>& expected) {
    if (actual.size() != expected.size()) return false;
    for (size_t i = 0; i < actual.size(); i++) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}

void dijkstraTests() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "        STARTING Dijkstra TEST          " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "[Dijkstra] Simple path check... ";
    Graph g1(3, true);
    g1.addEdge(0, 1, 10.0f);
    g1.addEdge(1, 2, 10.0f);

    auto path1 = dijkstra(g1, 0, 2);
    assert(verifyPath(path1.first, {0, 1, 2}));
    assert(path1.second == 20.0f);
    std::cout << "Passed." << std::endl;

    std::cout << "[Dijkstra] Shortcut check... ";
    Graph g2(3, true);
    g2.addEdge(0, 1, 10.0f);
    g2.addEdge(1, 2, 10.0f); 
    // Add a direct shortcut with lower cost
    g2.addEdge(0, 2, 5.0f);

    auto path2 = dijkstra(g2, 0, 2);
    assert(verifyPath(path2.first, {0, 2}));
    assert(path2.second == 5.0f);
    std::cout << "Passed." << std::endl;

    std::cout << "[Dijkstra] Unreachable check... ";
    Graph g3(4, true);
    g3.addEdge(0, 1, 5.0f);
    g3.addEdge(2, 3, 5.0f);
    // No connection between component {1,2} and {3,4}

    auto path3 = dijkstra(g3, 0, 3);
    assert(path3.first.empty());
    std::cout << "Passed." << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
    std::cout << "========================================" << std::endl;
}