#include <cassert>
#include <iostream>
#include <vector>

#include "tests.hpp"

/**
 * @brief Tests basic construction and initial state of the DSU.
 */
static void testConstructionAndBasics() {
  std::cout << "[Test] Construction and Basics..." << std::endl;

  int nNodes = 10;
  DSU dsu(nNodes);

  // 1. Check initial component count
  assert(dsu.components == nNodes);

  // 2. Check that every node is its own parent initially
  for (int i = 0; i < nNodes; ++i) {
    assert(dsu.parent[i] == i);
    assert(dsu.rank[i] == 0);
    assert(dsu.find(i) == i);
  }

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Tests the unite and find operations, including connectivity checks.
 */
static void testUnionAndFind() {
  std::cout << "[Test] Union and Find Operations..." << std::endl;

  DSU dsu(5);  // Nodes 0, 1, 2, 3, 4

  // 1. Unite 0 and 1
  bool merged = dsu.unite(0, 1);
  assert(merged == true);
  assert(dsu.isConnected(0, 1) == true);
  assert(dsu.components == 4);  // 5 - 1 = 4

  // 2. Unite 2 and 3
  dsu.unite(2, 3);
  assert(dsu.isConnected(2, 3) == true);
  assert(dsu.isConnected(0, 2) == false);  // Sets {0,1} and {2,3} are disjoint
  assert(dsu.components == 3);

  // 3. Unite the two sets ({0,1} and {2,3}) via 1-2
  merged = dsu.unite(1, 2);
  assert(merged == true);

  // Now 0, 1, 2, 3 should all be connected
  assert(dsu.isConnected(0, 3) == true);
  assert(dsu.isConnected(0, 2) == true);
  assert(dsu.components == 2);  // {0,1,2,3} and {4}

  // 4. Try to unite already connected nodes (should return false)
  merged = dsu.unite(0, 3);
  assert(merged == false);
  assert(dsu.components == 2);  // Count shouldn't change

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Tests the Path Compression optimization indirectly.
 * Logic: Construct a tall tree (line) and check if find() flattens it.
 */
static void testPathCompression() {
  std::cout << "[Test] Path Compression Logic..." << std::endl;

  int n = 5;
  DSU dsu(n);

  // Force a specific structure manually to simulate a chain 0->1->2->3->4
  // Note: unite() usually uses rank to prevent this, so we manually set parents
  // just to test if find() compresses correctly.
  dsu.parent[0] = 1;
  dsu.parent[1] = 2;
  dsu.parent[2] = 3;
  dsu.parent[3] = 4;
  dsu.parent[4] = 4;  // Root

  // Before find(0), parent of 0 is 1.
  assert(dsu.parent[0] == 1);

  // Execute find(0). This should traverse up to 4 and update all parents on
  // path.
  int root = dsu.find(0);
  assert(root == 4);

  // Check compression: parent of 0 should now be 4 directly.
  assert(dsu.parent[0] == 4);
  // Parent of 1 (on the path) should also be 4 now (recursive compression).
  assert(dsu.parent[1] == 4);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Tests the reset functionality (crucial for ROAR-NET performance).
 */
static void testReset() {
  std::cout << "[Test] Reset Functionality..." << std::endl;

  DSU dsu(5);

  // Mess up the state
  dsu.unite(0, 1);
  dsu.unite(1, 2);
  dsu.unite(3, 4);
  assert(dsu.components == 2);

  // Reset
  dsu.reset();

  // Validate initial state
  assert(dsu.components == 5);
  for (int i = 0; i < 5; ++i) {
    assert(dsu.parent[i] == i);
    assert(dsu.rank[i] == 0);
  }

  // Ensure connectivity is gone
  assert(dsu.isConnected(0, 1) == false);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Tests Union by Rank logic.
 * Ensures the smaller tree is attached to the larger tree root.
 */
static void testUnionByRank() {
  std::cout << "[Test] Union By Rank..." << std::endl;

  DSU dsu(4);

  // Make tree {0, 1} with root 1 (assuming 1 becomes root due to logic or
  // index) Initially rank 0 vs 0. Implementation usually picks one.
  dsu.unite(0, 1);
  int root01 = dsu.find(0);
  assert(dsu.rank[root01] == 1);  // Rank should increase to 1

  // Make element 2 independent (Rank 0)

  // Union {0,1} with {2}.
  // Rank of {0,1} is 1. Rank of {2} is 0.
  // {2} should become child of {0,1}'s root.
  dsu.unite(root01, 2);

  assert(dsu.find(2) == root01);  // 2 points to 1 (or 0)
  assert(dsu.rank[root01] == 1);  // Rank shouldn't increase (1 > 0)

  std::cout << " -> Passed." << std::endl;
}

void dsuTests() {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "        STARTING DSU TEST SUITE         " << std::endl;
  std::cout << "========================================" << std::endl;

  testConstructionAndBasics();
  testUnionAndFind();
  testPathCompression();
  testReset();
  testUnionByRank();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL DSU TESTS PASSED SUCCESSFULLY " << std::endl;
  std::cout << "========================================" << std::endl;
}
