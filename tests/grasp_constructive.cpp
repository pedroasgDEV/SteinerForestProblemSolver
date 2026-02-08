#include "tests.hpp"

/**
 * @brief Helper function to find the index of an edge (u -> v) in the Graph.
 */
int findEdgeIndex(const std::shared_ptr<Graph>& g, int u, int v) {
  for (int i = 0; i < g->nEdges; ++i)
    if (g->edges[i].source == u && g->edges[i].target == v) return i;
  return -1;
}

void testDisjointPaths() {
  std::cout << "[Test] Constructive: Disjoint Paths... ";

  // Two separate components, no sharing possible.
  // Nodes: 4
  // Edges: 0-1 (10.0), 2-3 (10.0), 1, 2, (1000)
  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 10.0f},
                                                       {2, 3, 10.0f},
                                                       {1, 2, 1000.0f}};
  int nNodes = 4;

  auto graph = std::make_shared<Graph>(edgeList, nNodes);
  std::vector<std::pair<int, int>> terminals = {{0, 1}, {2, 3}};

  // Create Problem Instance
  SFPProblem problem(graph, terminals);

  // Solver with Alpha 0 = Greedy
  GRASPConstructiveHeuristic solver(0.0f);
  SFPSolution solution = solver.solve(problem);

  // Verify Feasibility (Connectivity)
  DSU dsu(nNodes);
  assert(solution.isFeasible(dsu) == true);

  // Verify Specific Connections
  int idx01 = findEdgeIndex(graph, 0, 1);
  int idx23 = findEdgeIndex(graph, 2, 3);

  assert(idx01 != -1);
  assert(solution.isEdgeActive(idx01) == true);

  assert(idx23 != -1);
  assert(solution.isEdgeActive(idx23) == true);

  // Verify Total Cost
  // Expected: 10 + 10 = 20
  assert(std::abs(solution.getObjectiveValue() - 20.0f) < 0.001f);

  std::cout << "-> Passed." << std::endl;
}

void testAlphaRandomness() {
  std::cout << "[Test] Constructive: Alpha Randomness... ";

  // Disjoint paths with different weights
  // Edges: 0-1 (10.0), 2-3 (100.0)
  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 10.0f},
                                                       {2, 3, 100.0f},
                                                       {1, 2, 1000.0f}};
  int nNodes = 4;

  auto graph = std::make_shared<Graph>(edgeList, nNodes);
  std::vector<std::pair<int, int>> terminals = {{0, 1}, {2, 3}};

  SFPProblem problem(graph, terminals);

  // Solver with Alpha 1.0 = Pure Random
  // (In this simple graph, random choice still picks the only available edges)
  GRASPConstructiveHeuristic solver(1.0f);
  SFPSolution solution = solver.solve(problem);

  // 1. Verify Feasibility
  DSU dsu(nNodes);
  assert(solution.isFeasible(dsu) == true);

  // 2. Verify Total Cost
  // Expected: 10 + 100 = 110
  assert(std::abs(solution.getObjectiveValue() - 110.0f) < 0.001f);

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
