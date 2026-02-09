#include "tests.hpp"

/**
 * @brief Helper to find the specific index of a directed edge u->v
 */
int getEdgeIndex(const std::shared_ptr<Graph>& g, int u, int v) {
  for (int i = 0; i < g->nEdges; ++i) {
    if (g->edges[i].source == u && g->edges[i].target == v) return i;
  }
  return -1;
}

/**
 * @brief TEST 1: Optimization
 * Scenario: The initial solution uses a very expensive direct edge.
 * Local search must find a cheaper alternative path (detour).
 */
void testLocalSearchOptimization() {
  std::cout << "[Test] Local Search: Optimization (Shortcut)... ";

  // Graph: Triangle
  // 0 -> 1 : Weight 100 (Expensive direct link)
  // 0 -> 2 : Weight 10  (Cheap detour part 1)
  // 2 -> 1 : Weight 10  (Cheap detour part 2)
  std::vector<std::tuple<int, int, float>> edges = {
      {0, 1, 100.0f}, {0, 2, 10.0f}, {2, 1, 10.0f}};
  int nNodes = 3;
  auto g = std::make_shared<Graph>(edges, nNodes);

  // Terminals: 0 -> 1
  std::vector<std::pair<int, int>> terminals = {{0, 1}};
  SFPProblem problem(g, terminals);

  // Setup Initial Bad Solution: Only edge 0->1 active
  SFPSolution sol(problem);
  int idxBad = getEdgeIndex(g, 0, 1);

  // Manually apply move to set initial state
  SFPMove(MoveType::ADD, idxBad, 100.0f).apply(sol);

  assert(std::abs(sol.getObjectiveValue() - 100.0f) < 0.001f);

  // Execute Local Search
  GRASPLocalSearch ls;
  bool improved = ls.optimize(sol);

  // Validations
  assert(improved == true);

  // Expected Cost: 10 + 10 = 20
  assert(std::abs(sol.getObjectiveValue() - 20.0f) < 0.001f);

  // Validate Edges (Bad edge removed, detour added)
  int idx02 = getEdgeIndex(g, 0, 2);
  int idx21 = getEdgeIndex(g, 2, 1);

  assert(sol.isEdgeActive(idx02));
  assert(sol.isEdgeActive(idx21));
  assert(!sol.isEdgeActive(idxBad));

  // Validate Connectivity
  DSU dsu(nNodes);
  assert(sol.isFeasible(dsu));

  std::cout << "-> Passed." << std::endl;
}

void localSearchTests() {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "    Running Local Search Tests          " << std::endl;
  std::cout << "========================================" << std::endl;

  testLocalSearchOptimization();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
