#include "Tests.hpp"
#include <memory>

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
  DijkstraEngine engine1(std::make_shared<Graph>(g1));
  auto res1 = engine1.getShortPath(0, 2);

  // Expect: 0 -> 1 -> 2
  assert(verifyPath(g1, res1.first, {0, 1, 2}));
  assert(res1.second == 20.0f);
  std::cout << "-> Passed." << std::endl;

  std::cout << "[Dijkstra] Shortcut check... ";

  // 0-1 (10), 1-2 (10), 0-2 (5)
  Graph g2({{0, 1, 10.0f}, {1, 2, 10.0f}, {0, 2, 5.0f}}, 3);

  DijkstraEngine engine2(std::make_shared<Graph>(g2));
  auto res2 = engine2.getShortPath(0, 2);

  // Expect: 0 -> 2 (Direct path is cheaper)
  assert(verifyPath(g2, res2.first, {0, 2}));
  assert(res2.second == 5.0f);

  std::cout << "-> Passed." << std::endl;

  std::cout << "[Dijkstra] Unreachable check... ";

  // Graph: Disconnected components {0, 1} and {2, 3}
  Graph g3({{0, 1, 5.0f}, {2, 3, 5.0f}}, 4);

  DijkstraEngine engine3(std::make_shared<Graph>(g3));
  auto res3 = engine3.getShortPath(0, 3);

  // Expect: Empty path and cost -1 (or whatever your logic for infinity is)
  assert(res3.first.empty());
  assert(res3.second == -1.0f);

  std::cout << "-> Passed." << std::endl;

  std::cout << "[Dijkstra] Dynamic Obstacle (Ditch Bitmask)... ";

  Graph g4(
      {
          {0, 1, 10.0f},  
          {1, 2, 10.0f},  
          {0, 3, 50.0f},  
          {3, 2, 50.0f}   
      },
      4);

  DijkstraEngine engine4(std::make_shared<Graph>(g4));
  auto run1 = engine4.getShortPath(0, 2);

  assert(verifyPath(g4, run1.first, {0, 1, 2}));
  assert(run1.second == 20.0f);

  // Helper to find the actual CSR index of an edge
  auto getEdgeIdx = [&](int u, int v) {
      for (size_t i = 0; i < g4.edges.size(); ++i) {
          if (g4.edges[i].source == u && g4.edges[i].target == v) return i;
      }
      return (size_t)-1;
  };

  // ZERO-COPY: Block the cheap path
  std::vector<uint8_t> ditchMask(g4.edges.size(), 0);
  ditchMask[getEdgeIdx(0, 1)] = true; 

  auto run2 = engine4.getShortPath(0, 2, nullptr, &ditchMask);

  assert(verifyPath(g4, run2.first, {0, 3, 2}));
  assert(run2.second == 100.0f);

  std::cout << "-> Passed." << std::endl;

  std::cout << "[Dijkstra] Zero-Cost Path (Bridge Bitmask)... ";

  // ZERO-COPY: Make the expensive path completely free
  std::vector<uint8_t> bridgeMask(g4.edges.size(), 0);
  bridgeMask[getEdgeIdx(0, 3)] = true; 
  bridgeMask[getEdgeIdx(3, 2)] = true; 

  auto run3 = engine4.getShortPath(0, 2, &bridgeMask, nullptr);

  // Should take the now-free path (0-3-2)
  assert(verifyPath(g4, run3.first, {0, 3, 2}));
  assert(run3.second == 0.0f);

  std::cout << "-> Passed." << std::endl;

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
