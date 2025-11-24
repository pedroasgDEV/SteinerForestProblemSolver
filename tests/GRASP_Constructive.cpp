#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <cmath>

#include "../utils/graph.hpp"
#include "../algorithms/algorithms.hpp" 



void testDisjointPaths() {
    std::cout << "[Test] Constructive: Disjoint Paths... ";

    // Scenario: Two separate components, no sharing possible.
    Graph g(4, true);
    g.addEdge(0, 1, 10.0f);
    g.addEdge(2, 3, 10.0f);

    std::vector<std::pair<int, int>> terminals = {{0, 1}, {2, 3}};

    // Alpha 0 = Greedy
    Graph solution = GRASPconstructiveHeuristic(g, terminals, 0.0f);

    // Verify connections
    assert(solution.isAdjacent(0, 1) == true);
    assert(solution.isAdjacent(2, 3) == true);
    assert(solution.isAdjacent(0, 2) == false);

    // Verify total cost
    assert(std::abs(solution.getTotalWeight() - 20.0f) < 0.001f);

    std::cout << "Passed." << std::endl;
}

void testAlphaRandomness() {
    std::cout << "[Test] Constructive: Alpha Randomness... ";
    
    Graph g(4, true);
    g.addEdge(0, 1, 10.0f);
    g.addEdge(2, 3, 100.0f);
    
    std::vector<std::pair<int, int>> terminals = {{0, 1}, {2, 3}};
    
    Graph solution = GRASPconstructiveHeuristic(g, terminals, 1.0f);
    
    assert(solution.isReachable(0, 1));
    assert(solution.isReachable(2, 3));
    assert(solution.getTotalWeight() == 110.0f);

    std::cout << "Passed." << std::endl;
}

void GRASPconstructiveTests() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  Running Constructive Heuristic Tests  " << std::endl;
    std::cout << "========================================" << std::endl;
    
    testDisjointPaths();
    testAlphaRandomness();

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
    std::cout << "========================================" << std::endl;
}