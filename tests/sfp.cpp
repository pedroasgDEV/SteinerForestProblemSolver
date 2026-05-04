#include "Tests.hpp"

#include "../models/SFP.hpp"

#include <sstream>

/**
 * @brief Helper to find an edge index given its source and target.
 */
int findEdgeIndex(const Graph& g, int u, int v) {
  for (size_t i = 0; i < g.edges.size(); ++i)
    if (g.edges[i].source == u && g.edges[i].target == v) return i;
  return -1;
}

/**
 * @brief Test 1: Basic Problem Construction and Empty Solution
 */
void testBasicConstruction() {
  std::cout << "[Test] Basic Construction...";

  std::vector<std::tuple<int, int, float>> edgeList = {
      {0, 1, 10.0f}, {1, 2, 10.0f}, {2, 0, 10.0f}};
  int nNodes = 3;

  auto graph = std::make_shared<Graph>(edgeList, nNodes);
  std::vector<std::pair<int, int>> terminals = {{0, 1}};

  SFPProblem problem(graph, terminals);
  problem.setName("TriangleTest");

  assert(problem.getNNodes() == 3);
  assert(problem.getNEdges() == 6);  // 3 edges * 2 directions
  assert(problem.getTerminals().size() == 1);

  SFPSolution sol = problem.empty_solution();
  assert(sol.getCurrentCost() == 0.0f);
  assert(sol.isFeasible() == false);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 2: Connect, Disconnect and Undo Logic
 */
void testConnectAndDisconnectMoves() {
  std::cout << "[Test] Connect, Disconnect & Undo Moves...";

  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 10.0f}, {1, 2, 20.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 3);
  SFPProblem problem(graph, {{0, 2}});
  SFPSolution sol = problem.empty_solution();

  int pair_id = 0; // O pair_id gerado para {0, 2}

  int e01 = findEdgeIndex(*graph, 0, 1);
  int e12 = findEdgeIndex(*graph, 1, 2);

  SFPMove connectMove(&sol, MoveType::CNCT_PAIR, pair_id, {e01, e12});

  // 1. Test the Connection Apply
  connectMove.apply();
  assert(sol.getCurrentCost() == 30.0f);
  assert(sol.getEdges().size() == 2);
  assert(sol.getPairEdges(pair_id).size() == 2);
  assert(sol.getPairCost(pair_id) == 30.0f);

  // 2. Test the Connection Undo
  connectMove.undo();
  assert(sol.getCurrentCost() == 0.0f);
  assert(sol.getEdges().size() == 0);
  assert(sol.getPairEdges(pair_id).size() == 0);
  assert(sol.getPairCost(pair_id) == 0.0f);

  // 3. Apply again so we can test the Disconnection
  connectMove.apply();
  
  SFPMove disconnectMove(&sol, MoveType::DSCNCT_PAIR, pair_id, {e01, e12});
  
  // 4. Test the Disconnection
  disconnectMove.apply();
  assert(sol.getCurrentCost() == 0.0f);
  assert(sol.getEdges().size() == 0);
  assert(sol.getPairEdges(pair_id).size() == 0);
  assert(sol.getPairCost(pair_id) == 0.0f);

  // 5. Test the Disconnection Undo
  disconnectMove.undo();
  assert(sol.getCurrentCost() == 30.0f);
  assert(sol.getEdges().size() == 2);
  assert(sol.getPairEdges(pair_id).size() == 2);
  assert(sol.getPairCost(pair_id) == 30.0f);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 3: Neighborhood (Batch Moves) with Heap Ownership
 */
void testNeighborhoods() {
  std::cout << "[Test] Neighborhoods (Batch Moves)...";

  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 5.0f}, {1, 2, 8.0f}, {2, 3, 10.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 4);
  SFPProblem problem(graph, {{0, 1}, {2, 3}});
  SFPSolution sol = problem.empty_solution();

  int p1_id = 0;
  int p2_id = 1;

  int e01 = findEdgeIndex(*graph, 0, 1);
  int e23 = findEdgeIndex(*graph, 2, 3);

  SFPNeighborhood neigh;
  neigh.addMoveApplying(SFPMove(&sol, MoveType::CNCT_PAIR, p1_id, {e01}));
  neigh.addMoveApplying(SFPMove(&sol, MoveType::CNCT_PAIR, p2_id, {e23}));

  // Test Batch Application
  assert(sol.getCurrentCost() == 15.0f); // 5.0 + 10.0
  assert(sol.getEdges().size() == 2);

  // Test Batch Undo
  neigh.undo();
  assert(sol.getCurrentCost() == 0.0f);
  assert(sol.getEdges().size() == 0);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 4: Feasibility Logic with DSU
 */
void testFeasibility() {
  std::cout << "[Test] Feasibility Logic...";

  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 5.0f}, {1, 2, 5.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 3);
  SFPProblem problem(graph, {{0, 2}}); // The terminal is {0, 2}
  SFPSolution sol = problem.empty_solution();

  assert(sol.isFeasible() == false);

  int p1_id = 0;
  int e01 = findEdgeIndex(*graph, 0, 1);
  int e12 = findEdgeIndex(*graph, 1, 2);

  // Connect the entire path of the terminal {0, 2}
  SFPMove connectMove(&sol, MoveType::CNCT_PAIR, p1_id, {e01, e12});
  connectMove.apply();

  // Now the DSU should find the path 0 -> 1 -> 2
  assert(sol.isFeasible() == true);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 5: IO Parsing
 */
void testIOParsing() {
  std::cout << "[Test] IO Parsing (>> <<)... ";

  std::string inputData = R"(
        SECTION Graph
        Nodes 4
        Edges 3
        E 1 2 10
        E 2 3 20
        E 3 4 30
        END
        
        SECTION Terminals
        Terminals 1
        TP 1 4
        END
    )";

  std::stringstream ss(inputData);
  SFPProblem problem;
  ss >> problem;

  assert(problem.getNNodes() == 4);
  assert(problem.getNEdges() == 6);
  assert(problem.getTerminals().size() == 1);

  std::cout << problem << std::endl;
  
  SFPSolution sol = problem.empty_solution();
  int testPairId = 0;
  
  int e12 = findEdgeIndex(*problem.getGraphPtr(), 0, 1);
  
  // Perform a test move with the parsed graph
  SFPMove mv(&sol, MoveType::CNCT_PAIR, testPairId, {e12});
  mv.apply();

  assert(sol.getCurrentCost() == 10.0f);
  
  std::cout << sol << std::endl;
  
  std::cout << " -> Passed." << std::endl;
}

void steinerForestTests() {
  std::cout << "========================================" << std::endl;
  std::cout << "         STARTING SFP TEST SUITE        " << std::endl;
  std::cout << "========================================" << std::endl;

  testBasicConstruction();
  testConnectAndDisconnectMoves();
  testNeighborhoods();
  testFeasibility();
  testIOParsing();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
