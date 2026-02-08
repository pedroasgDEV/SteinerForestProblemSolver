#include <sstream>

#include "tests.hpp"

/**
 * @brief Helper to find an edge index given source and target.
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

  // Create a Triangle Graph: 0-1 (10), 1-2 (10), 2-0 (10)
  std::vector<std::tuple<int, int, float>> edgeList = {
      {0, 1, 10.0f}, {1, 2, 10.0f}, {2, 0, 10.0f}};
  int nNodes = 3;

  auto graph = std::make_shared<Graph>(edgeList, nNodes);

  // Terminals: 0 and 1
  std::vector<std::pair<int, int>> terminals = {{0, 1}};

  SFPProblem problem(graph, terminals);
  problem.setName("TriangleTest");

  // Check Problem Properties
  assert(problem.getNNodes() == 3);
  assert(problem.getNEdges() == 6);  // 3 edges * 2 directions
  assert(problem.getTerminals().size() == 1);

  // Check Empty Solution
  SFPSolution sol = problem.empty_solution();
  assert(sol.getObjectiveValue() == 0.0f);

  // Check Feasibility of Empty Solution (Should be false, terminals
  // disconnected)
  DSU dsu(nNodes);
  assert(sol.isFeasible(dsu) == false);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 2: Move Mechanics (Apply/Undo and Cost Consistency)
 */
void testMovesAndCost() {
  std::cout << "[Test] Move Mechanics (Apply/Undo)...";

  // Line Graph: 0 --(10)--> 1 --(20)--> 2
  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 10.0f},
                                                       {1, 2, 20.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 3);
  std::vector<std::pair<int, int>> terminals = {{0, 2}};
  SFPProblem problem(graph, terminals);

  SFPSolution sol = problem.empty_solution();

  // Find index for edge 0->1
  int idx01 = findEdgeIndex(*graph, 0, 1);
  int idx10 = findEdgeIndex(*graph, 1, 0);  // Reverse
  assert(idx01 != -1 && idx10 != -1);

  // --- TEST ADD MOVE ---
  // Create Move: Add 0->1 (Cost 10)
  SFPMove moveAdd(MoveType::ADD, idx01, 10.0f);

  moveAdd.apply(sol);

  // Checks:
  // 1. Cost updated
  assert(sol.getObjectiveValue() == 10.0f);
  // 2. Edge active
  assert(sol.isEdgeActive(idx01) == true);
  // 3. Reverse edge synced (Bidirectional Logic)
  assert(sol.isEdgeActive(idx10) == true);

  // --- TEST UNDO ADD ---
  moveAdd.undo(sol);

  assert(sol.getObjectiveValue() == 0.0f);
  assert(sol.isEdgeActive(idx01) == false);
  assert(sol.isEdgeActive(idx10) == false);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 3: Feasibility Logic with DSU
 */
void testFeasibility() {
  std::cout << "[Test] Feasibility Logic...";

  // Path: 0 -> 1 -> 2. Terminals: (0, 2)
  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 5.0f},
                                                       {1, 2, 5.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 3);
  SFPProblem problem(graph, {{0, 2}});
  SFPSolution sol = problem.empty_solution();
  DSU dsu(3);

  // 1. Empty -> Not Feasible
  assert(sol.isFeasible(dsu) == false);

  // 2. Add 0->1 only -> Not Feasible (0 connected to 1, but not 2)
  int idx01 = findEdgeIndex(*graph, 0, 1);
  SFPMove m1(MoveType::ADD, idx01, 5.0f);
  m1.apply(sol);
  assert(sol.isFeasible(dsu) == false);

  // 3. Add 1->2 -> Feasible (0-1-2 connected)
  int idx12 = findEdgeIndex(*graph, 1, 2);
  SFPMove m2(MoveType::ADD, idx12, 5.0f);
  m2.apply(sol);
  assert(sol.isFeasible(dsu) == true);

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 4: Random Constructive Heuristic
 */
void testRandomSolution() {
  std::cout << "[Test] Random Solution Heuristic...";

  // Square Graph 0-1-2-3-0. All weights 1. Terminals (0, 2).
  // Optimal path is length 2 (0-1-2 or 0-3-2).
  std::vector<std::tuple<int, int, float>> edgeList = {
      {0, 1, 1.0f}, {1, 2, 1.0f}, {2, 3, 1.0f}, {3, 0, 1.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 4);
  SFPProblem problem(graph, {{0, 2}});

  // Run multiple times to verify robustness
  for (int i = 0; i < 5; ++i) {
    SFPSolution sol = problem.random_solution();
    DSU dsu(4);

    // Must be feasible
    assert(sol.isFeasible(dsu) == true);

    // Cost must be positive (at least 2.0f for path of length 2)
    assert(sol.getObjectiveValue() >= 2.0f);
  }

  std::cout << " -> Passed." << std::endl;
}

/**
 * @brief Test 5: Neighborhood Generation (Add/Remove)
 */
void testNeighborhoods() {
  std::cout << "[Test] Neighborhood Generation...";

  // Graph 0-1 (10). One edge.
  std::vector<std::tuple<int, int, float>> edgeList = {{0, 1, 10.0f}};
  auto graph = std::make_shared<Graph>(edgeList, 2);
  SFPProblem problem(graph,
                     {{0, 1}});  // Terminals irrelevant for move generation

  SFPSolution sol = problem.empty_solution();

  // --- Test AddNeighbourhood (on empty solution) ---
  AddNeighbourhood addNH(problem);
  std::vector<SFPMove> moves = addNH.moves(sol);

  // Should generate EXACTLY 1 move (for 0->1).
  // Should NOT generate move for 1->0 because of canonical check (source <
  // target).
  assert(moves.size() == 1);
  assert(moves[0].type == MoveType::ADD);
  assert(moves[0].costDelta == 10.0f);

  // Apply the move to test RemoveNH
  moves[0].apply(sol);
  assert(sol.getObjectiveValue() == 10.0f);

  // --- Test RemoveNeighbourhood (on full solution) ---
  RemoveNeighbourhood remNH(problem);
  std::vector<SFPMove> remMoves = remNH.moves(sol);

  // Should generate EXACTLY 1 move
  assert(remMoves.size() == 1);
  assert(remMoves[0].type == MoveType::REMOVE);
  assert(remMoves[0].costDelta == -10.0f);  // Negative delta

  // --- Test AddNeighbourhood (on full solution) ---
  // Should generate 0 moves because the edge is already active
  std::vector<SFPMove> addMovesEmpty = addNH.moves(sol);
  assert(addMovesEmpty.empty());

  std::cout << " -> Passed." << std::endl;
}

void testIOParsing() {
  std::cout << "[Test] IO Parsing (>> <<)... " << std::endl;

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
  assert(problem.getTerminals()[0].first == 0);
  assert(problem.getTerminals()[0].second == 3);

  assert(problem.getGraphPtr()->totalWeight == 60.0f);

  std::cout << "\n--- Problem Print Output ---" << std::endl;
  std::cout << problem << std::endl;

  SFPSolution sol = problem.random_solution();

  std::cout << "--- Solution Print Output (Random Valid) ---" << std::endl;
  std::cout << sol << std::endl;

  DSU dsu(problem.getNNodes());
  assert(sol.isFeasible(dsu) == true);
  assert(sol.getObjectiveValue() > 0);
}

void steinerForestTests() {
  std::cout << "========================================" << std::endl;
  std::cout << "         STARTING SFP TEST SUITE        " << std::endl;
  std::cout << "========================================" << std::endl;

  testBasicConstruction();
  testMovesAndCost();
  testFeasibility();
  testRandomSolution();
  testNeighborhoods();
  testIOParsing();

  std::cout << "========================================" << std::endl;
  std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
  std::cout << "========================================" << std::endl;
}
