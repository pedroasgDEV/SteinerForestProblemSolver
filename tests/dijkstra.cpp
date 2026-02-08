#include "tests.hpp"

// Helper function to verify if the list of EDGE INDICES corresponds to the
// sequence of NODES
bool verifyPath(const Graph& g, const std::vector<int>& pathEdges,
                const std::vector<int>& expectedNodes) {
  // Path is empty
  if (pathEdges.empty()) return expectedNodes.size() <= 1;

  // Mismatch in size
  if (pathEdges.size() != expectedNodes.size() - 1) return false;

  auto temp = pathEdges;
  std::reverse(temp.begin(), temp.end());
  int expectedSource = expectedNodes[0];

  // Iterate through edge indices returned by Dijkstra
  for (size_t i = 0; i < temp.size(); ++i) {
    int edgeIdx = temp[i];
    const auto& edge = g.edges[edgeIdx];  // Get the actual edge from graph

    if (edge.source != expectedSource)
      return false;  // Verify if edge starts where the previous one ended

    if (edge.target != expectedNodes[i + 1])
      return false;  // Verify if edge points to the correct next node

    expectedSource = edge.target;
  }

  return true;
}

void dijkstraTests() {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "          STARTING DIJKSTRA TEST        " << std::endl;
  std::cout << "========================================" << std::endl;

  std::cout << "[Dijkstra] Simple path check... ";

  // Graph: 0-1 (10), 1-2 (10)
  Graph g1({{0, 1, 10.0f}, {1, 2, 10.0f}}, 3);

  // Setup Engine for 3 nodes
  DijkstraEngine engine1(3);
  auto res1 = engine1.getShortPath(g1, 0, 2);

  // Expect: 0 -> 1 -> 2
  assert(verifyPath(g1, res1.first, {0, 1, 2}));
  assert(res1.second == 20.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "[Dijkstra] Shortcut check... ";

  // 0-1 (10), 1-2 (10), 0-2 (5)
  Graph g2({{0, 1, 10.0f}, {1, 2, 10.0f}, {0, 2, 5.0f}}, 3);

  DijkstraEngine engine2(3);
  auto res2 = engine2.getShortPath(g2, 0, 2);

  // Expect: 0 -> 2 (Direct path is cheaper)
  assert(verifyPath(g2, res2.first, {0, 2}));
  assert(res2.second == 5.0f);

  std::cout << "Passed." << std::endl;

  std::cout << "[Dijkstra] Unreachable check... ";

  // Graph: Disconnected components {0, 1} and {2, 3}
  Graph g3({{0, 1, 5.0f}, {2, 3, 5.0f}}, 4);

  DijkstraEngine engine3(4);
  auto res3 = engine3.getShortPath(g3, 0, 3);

  // Expect: Empty path and cost -1 (or whatever your logic for infinity is)
  assert(res3.first.empty());
  // Note: Verify if your implementation returns -1.0f or Infinity for
  // unreachable
  assert(res3.second == -1.0f);

  std::cout << "Passed." << std::endl;

  std::cout << "[Dijkstra] Dynamic Obstacle (Soft Deletion)... ";

  // Graph: Two paths from 0 to 2
  // Path A: 0 -> 1 -> 2 (Cost 20)
  // Path B: 0 -> 3 -> 2 (Cost 100)
  Graph g4(
      {
          {0, 1, 10.0f},
          {1, 2, 10.0f},  // Cheap path
          {0, 3, 50.0f},
          {3, 2, 50.0f}  // Expensive path
      },
      4);

  DijkstraEngine engine4(4);
  auto run1 = engine4.getShortPath(g4, 0, 2);

  // Should take cheap path (0-1-2)
  assert(verifyPath(g4, run1.first, {0, 1, 2}));
  assert(run1.second == 20.0f);

  // Block the cheap path (remove edge 0-1)
  Graph g5(
      {
          {1, 2, 10.0f},  // Cheap path
          {0, 3, 50.0f},
          {3, 2, 50.0f}  // Expensive path
      },
      4);
  auto run2 = engine4.getShortPath(g5, 0, 2);

  // Should take expensive path (0-3-2)
  assert(verifyPath(g5, run2.first, {0, 3, 2}));
  assert(run2.second == 100.0f);

  std::cout << "Passed." << std::endl;

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
