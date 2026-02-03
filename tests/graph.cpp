#include <cassert>
#include <iostream>
#include <ostream>
#include <vector>

#include "tests.hpp"

static void testConstructionAndBasics() {
  std::cout << "[Test] Construction and Basics..." << std::endl;

  // Graph: 0-1 (10), 1-2 (5)
  auto g = Graph({{0, 1, 10.0f}, {1, 2, 5.0f}}, 3, true);

  // Basic Checks
  assert(g.getNNodes() == 3);
  assert(g.getNEdges() == 4);  // 2 edges * 2 directions = 4 physical edges
  assert(g.getTotalWeight() == 15.0f);  // (10+10+5+5)/2 = 15

  // Verify CSR content (Row Pointers)
  // Node 0 has 1 neighbor -> ptr[0]=0, ptr[1]=1
  // Node 1 has 2 neighbors -> ptr[1]=1, ptr[2]=3
  // Node 2 has 1 neighbor -> ptr[2]=3, ptr[3]=4
  std::vector<int> expectedPtr = {0, 1, 3, 4};
  assert(g.getPtrs() == expectedPtr);

  std::cout << " -> Passed." << std::endl;
}

static void testConstraintFunctions() {
  std::cout << "[Test] Constraint Functions..." << std::endl;

  // Positive Connected Graph: 0-1, 1-2
  auto g = Graph({{0, 1, 10.0f}, {1, 2, 5.0f}}, 3, true);

  // Test 1: Normal Graph
  assert(hasNegativeWeights(g) == false);  // No negative weights
  assert(isGraphConnected(g) == true);     // Is connected

  // Test 2: Break connectivity (Deactivate edge 0-1)
  g.setEdgeStatus(g.getEdge(0, 1), false);
  assert(isGraphConnected(g) == false);  // Node 0 became isolated

  // Test 3: Negative Weights
  auto gNeg = Graph({{0, 1, -10.0f}}, 2, true);
  assert(hasNegativeWeights(gNeg) == true);

  std::cout << " -> Passed." << std::endl;
}

static void testActiveInactiveEdge() {
  std::cout << "[Test] Active/Inactive Edges (Soft Deletion)..." << std::endl;

  // Triangle Graph: 0-1 (10), 1-2 (20), 0-2 (30)
  auto g = Graph({{0, 1, 10.0f}, {1, 2, 20.0f}, {0, 2, 30.0f}}, 3, true);

  float initialWeight = 10 + 20 + 30;
  assert(g.getTotalWeight() == initialWeight);

  // 1. Remove edge 0-1
  g.setEdgeStatus(g.getEdge(0, 1), false);

  // Weight must decrease by 10
  assert(g.getTotalWeight() == (initialWeight - 10.0f));

  // BFS should not find path via 0-1 anymore (but might find via 0-2-1)
  // Here we trust the weight check as proof of deactivation.

  // Reactivate
  g.setEdgeStatus(g.getEdge(0, 1), true);
  assert(g.getTotalWeight() == initialWeight);

  std::cout << " -> Passed." << std::endl;
}

static void testUnidirectional() {
  std::cout << "[Test] Unidirectional Graph..." << std::endl;

  // 0 -> 1 (one way only)
  auto g = Graph({{0, 1, 10.0f}}, 2, false);

  assert(g.getNEdges() == 1);  // Only one physical edge
  assert(g.isReachable(0, 1) == true);
  assert(g.isReachable(1, 0) == false);  // Return path does not exist

  std::cout << " -> Passed." << std::endl;
}

static void testReachability() {
  std::cout << "[Test] Reachability (BFS)..." << std::endl;

  // Line Graph: 0 - 1 - 2    3 (isolated)
  auto g = Graph({{0, 1, 1.0f}, {1, 2, 1.0f}}, 4, true);

  // Positive Tests
  assert(g.isReachable(0, 1) == true);
  assert(g.isReachable(0, 2) == true);  // Transitive
  assert(g.isReachable(2, 0) == true);

  // Negative Tests
  assert(g.isReachable(0, 3) == false);
  assert(g.isReachable(1, 3) == false);

  // Dynamic Test: Cut the path in the middle
  g.setEdgeStatus(g.getEdge(0, 1), false);
  assert(g.isReachable(0, 2) == false);  // 0 reaches 1, but 1 does not reach 2

  std::cout << " -> Passed." << std::endl;
}

static void testPrint() {
  std::cout << "[Test] Printing (Visual Check)..." << std::endl;
  auto g = Graph({{0, 1, 1.5f}, {1, 2, 2.5f}}, 3, true);
  std::cout << g << std::endl;
  std::cout << " -> Passed." << std::endl;
}

void graphTests() {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "      STARTING CSR GRAPH TEST SUITE     " << std::endl;
  std::cout << "========================================" << std::endl;

  testConstructionAndBasics();
  testConstraintFunctions();
  testActiveInactiveEdge();
  testUnidirectional();
  testReachability();
  testPrint();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
