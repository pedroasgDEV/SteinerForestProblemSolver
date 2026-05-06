#include "Tests.hpp"
#include "../utils/BidDijkstra.hpp"
#include <iostream>
#include <memory>
#include <cassert>

/**
 * @brief Helper to verify if the edge indices returned by the engine
 * link the expected sequence of nodes.
 */
static bool verifyPath(const Graph& g, const std::vector<int>& pathEdges,
                const std::vector<int>& expectedNodes) {
  if (pathEdges.empty()) return expectedNodes.size() <= 1;
  if (pathEdges.size() != expectedNodes.size() - 1) return false;

  int expectedSource = expectedNodes[0];

  // Bidirectional reconstruction usually returns edges in Source->Target order
  for (size_t i = 0; i < pathEdges.size(); ++i) {
    int edgeIdx = pathEdges[i];
    const auto& edge = g.edges[edgeIdx];

    if (edge.source != expectedSource || edge.target != expectedNodes[i + 1])
      return false;

    expectedSource = edge.target;
  }
  return true;
}

void BidirectionalDijkstraTests() {
  std::cout << "\n========================================" << std::endl;
  std::cout << "      STARTING BIDIRECTIONAL DIJKSTRA TEST    " << std::endl;
  std::cout << "========================================" << std::endl;

  // Helper lambda to find edge index for mask testing
  auto getEdgeIdx = [](const Graph& g, int u, int v) {
      for (int i = 0; i < (int)g.edges.size(); ++i) {
          if (g.edges[i].source == u && g.edges[i].target == v) return i;
      }
      return -1;
  };

  std::cout << "[Bi-DIJKSTRA] Simple Path... ";
  // 0-1 (10), 1-2 (10)
  Graph g1({{0, 1, 10.0f}, {1, 2, 10.0f}}, 3);
  auto engine1 = std::make_shared<BidirectionalDijkstraEngine>(std::make_shared<Graph>(g1));
  auto res1 = engine1->getShortPath(0, 2);
  assert(verifyPath(g1, res1.first, {0, 1, 2}));
  assert(res1.second == 20.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "[Bi-DIJKSTRA] Shortcut (Dense check)... ";
  // 0-1 (10), 1-2 (10), 0-2 (5)
  Graph g2({{0, 1, 10.0f}, {1, 2, 10.0f}, {0, 2, 5.0f}}, 3);
  auto engine2 = std::make_shared<BidirectionalDijkstraEngine>(std::make_shared<Graph>(g2));
  auto res2 = engine2->getShortPath(0, 2);
  assert(verifyPath(g2, res2.first, {0, 2}));
  assert(res2.second == 5.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "[Bi-DIJKSTRA] Unreachable... ";
  // Components {0,1} and {2,3}
  Graph g3({{0, 1, 5.0f}, {2, 3, 5.0f}}, 4);
  auto engine3 = std::make_shared<BidirectionalDijkstraEngine>(std::make_shared<Graph>(g3));
  auto res3 = engine3->getShortPath(0, 3);
  assert(res3.first.empty());
  assert(res3.second == -1.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "[Bi-DIJKSTRA] Ditch Mask (Obstacle)... ";
  Graph g4({{0, 1, 10.0f}, {1, 2, 10.0f}, {0, 3, 50.0f}, {3, 2, 50.0f}}, 4);
  auto engine4 = std::make_shared<BidirectionalDijkstraEngine>(std::make_shared<Graph>(g4));
  
  std::vector<uint8_t> ditchMask(g4.edges.size(), 0);
  int blockedEdge = getEdgeIdx(g4, 0, 1);
  ditchMask[blockedEdge] = 1;
  // Also block the reverse direction for undirected consistency
  ditchMask[g4.edges[blockedEdge].reverseEdgePtr] = 1;

  auto res4 = engine4->getShortPath(0, 2, nullptr, &ditchMask);
  // Must take the expensive path 0-3-2 because 0-1 is blocked
  assert(verifyPath(g4, res4.first, {0, 3, 2}));
  assert(res4.second == 100.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "[Bi-DIJKSTRA] Bridge Mask (Zero-cost)... ";
  std::vector<uint8_t> bridgeMask(g4.edges.size(), 0);
  int freeEdge1 = getEdgeIdx(g4, 0, 3);
  int freeEdge2 = getEdgeIdx(g4, 3, 2);
  bridgeMask[freeEdge1] = 1;
  bridgeMask[freeEdge2] = 1;
  bridgeMask[g4.edges[freeEdge1].reverseEdgePtr] = 1;
  bridgeMask[g4.edges[freeEdge2].reverseEdgePtr] = 1;

  auto res5 = engine4->getShortPath(0, 2, &bridgeMask, nullptr);
  assert(verifyPath(g4, res5.first, {0, 3, 2}));
  assert(res5.second == 0.0f);
  std::cout << "Passed." << std::endl;

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL BI-DIJKSTRA TESTS PASSED            " << std::endl;
  std::cout << "========================================" << std::endl;
}
