#include <iostream>
#include <vector>
#include <cassert>

#include "tests.hpp"
#include "../utils/graph.hpp"

static void testConstructionAndBasics() {
    std::cout << "[Test] Construction and Basics..." << std::endl;
    
    // Test empty constructor
    Graph g1(5);
    assert(g1.getNNode() == 5);
    assert(g1.getIsBidirectional() == true);
    
    // Test constructor with matrix
    std::vector<std::vector<float>> inputMat = {
        {0, 10, 0},
        {10, 0, 5},
        {0, 5, 0}
    };
    Graph g2(inputMat, true);
    assert(g2.getNNode() == 3);
    
    // Verify matrix content 
    assert(g2.getMatrix() == inputMat);
    
    std::cout << " -> Passed." << std::endl;
}

static void testConstraintFunctions() {
    std::cout << "[Test] Constraint Functions..." << std::endl;
    
    // Test true return
    std::vector<std::vector<float>> inputMat = {
        {0, 10, 0},
        {10, 0, 5},
        {0, 5, 0}
    };
    Graph g2(inputMat, true);
    assert(hasNegativeWeights(g2) == true);
    assert(isGraphConnected(g2) == true);

    // Test false to connect constraint
    g2.removeEdge(0, 1);
    assert(hasNegativeWeights(g2) == true);
    assert(isGraphConnected(g2) == false);

    // Test false do negative weights
    g2.addEdge(0, 1, -10);
    assert(hasNegativeWeights(g2) == false);
    assert(isGraphConnected(g2) == true);

    std::cout << " -> Passed." << std::endl;
}

static void testAddRemoveEdge() {
    std::cout << "[Test] Add and Remove Edges..." << std::endl;

    Graph g(3, true);

    g.addEdge(0, 1, 5.5f);

    assert(g.getTotalWeight() == 5.5f);
    
    // Check adjacency
    assert(g.isAdjacent(0, 1) == true);
    assert(g.isAdjacent(1, 0) == true);
    assert(g.isAdjacent(0, 2) == false);

    // Check the matrix directly
    auto mat = g.getMatrix();
    assert(mat[0][1] == 5.5f);

    // Remove the edge
    g.removeEdge(0, 1);
    assert(g.isAdjacent(0, 1) == false);

    // Check the total of weights
    assert(g.getTotalWeight() == 0);

    std::cout << " -> Passed." << std::endl;
}

static void testUnidirectional() {
    std::cout << "[Test] Unidirectional Graph..." << std::endl;

    Graph g(3, false); 
    
    g.addEdge(0, 1, 10.0f);

    assert(g.isAdjacent(0, 1) == true);
    assert(g.isAdjacent(1, 0) == false);

    std::cout << " -> Passed." << std::endl;
}

static void testReachability() {
    std::cout << "[Test] Reachability (BFS)..." << std::endl;

    Graph g(4, true);
    g.addEdge(0, 1);
    g.addEdge(1, 2);

    // Positive tests
    assert(g.isReachable(0, 1) == true);
    assert(g.isReachable(0, 2) == true); 
    assert(g.isReachable(2, 0) == true);

    // Negative tests
    assert(g.isReachable(0, 3) == false);
    assert(g.isReachable(1, 3) == false);

    std::cout << " -> Passed." << std::endl;
}

static void testCopyAndEquality() {
    std::cout << "[Test] Deep Copy and Operators..." << std::endl;

    Graph g1(3, true);
    g1.addEdge(0, 1, 10.0f);

    // Verify Deep Copy
    Graph g2(g1); 
    assert(g1 == g2); 
    g1.addEdge(1, 2, 5.0f);
    assert(g2.isAdjacent(1, 2) == false); 
    assert(g1 != g2); 

    // Assignment Operator
    Graph g3(10);
    g3 = g1;
    assert(g3 == g1);
    assert(g3.isAdjacent(1, 2) == true);

    std::cout << " -> Passed." << std::endl;
}

static void testPrint() {
    std::cout << "[Test] Printing (Visual Check)..." << std::endl;
    Graph g(3);
    g.addEdge(0, 1, 1.5f);
    std::cout << g << std::endl; 
    std::cout << " -> Passed." << std::endl;
}

void graphTests() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "      STARTING GRAPH TEST SUITE         " << std::endl;
    std::cout << "========================================" << std::endl;

    testConstructionAndBasics();
    testConstraintFunctions();
    testAddRemoveEdge();
    testUnidirectional();
    testReachability();
    testCopyAndEquality();
    testPrint();

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
    std::cout << "========================================" << std::endl;
}