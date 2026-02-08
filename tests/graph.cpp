#include <cassert>
#include <iostream>
#include <ostream>
#include <vector>

#include "tests.hpp"

static void testConstructionAndBasics() {
  std::cout << "[Test] Construction and Basics..." << std::endl;

  // Graph: 0-1 (10), 1-2 (5)
  auto g = Graph({{0, 1, 10.0f}, {1, 2, 5.0f}}, 3);

  // Basic Checks
  assert(g.nNodes == 3);
  assert(g.nEdges == 4);
  assert(g.totalWeight == 15.0f);  // (10+10+5+5)/2 = 15

  // Verify CSR content (Row Pointers)
  // Node 0 has 1 neighbor -> ptr[0]=0, ptr[1]=1
  // Node 1 has 2 neighbors -> ptr[1]=1, ptr[2]=3
  // Node 2 has 1 neighbor -> ptr[2]=3, ptr[3]=4
  std::vector<int> expectedPtr = {0, 1, 3, 4};
  assert(g.ptrs == expectedPtr);

  std::cout << " -> Passed." << std::endl;
}

static void testConstraintFunctions() {
  std::cout << "[Test] Constraint Functions..." << std::endl;

  // Positive Connected Graph: 0-1, 1-2
  auto g = Graph({{0, 1, 10.0f}, {1, 2, 5.0f}}, 3);

  // Test 1: Normal Graph
  assert(hasNegativeWeights(g) == false);  // No negative weights
  assert(isGraphConnected(g) == true);     // Is connected

  // Test 2: Break connectivity (Deactivate edge 0-1)
  auto g2 = Graph({{1, 2, 5.0f}}, 3);
  assert(isGraphConnected(g2) == false);  // Node 0 became isolated

  // Test 3: Negative Weights
  auto gNeg = Graph({{0, 1, -10.0f}}, 2);
  assert(hasNegativeWeights(gNeg) == true);

  std::cout << " -> Passed." << std::endl;
}

static void testPrint() {
  std::cout << "[Test] Printing (Visual Check)..." << std::endl;
  auto g = Graph({{0, 1, 1.5f}, {1, 2, 2.5f}}, 3);
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
  testPrint();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
