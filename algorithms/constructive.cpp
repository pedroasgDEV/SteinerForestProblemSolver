#include "Solver.hpp"

/**
 * @brief Internal auxiliary structure to manage the Candidate List (CL).
 */
struct CandidatePair {
  int source;
  int target;
  float currentPathCost;  ///< Cost calculated on the working graph (dynamic
                          ///< weights)

  /**
   * @brief Operator for sorting the CL (Ascending order of cost).
   */
  bool operator<(const CandidatePair& other) const {
    return currentPathCost < other.currentPathCost;
  }
};

/**
 * @brief Agglutinates terminal sets that share vertices.
 */
static std::vector<std::vector<int>> preprocessTerminalGroups(
    int nNodes, const std::vector<std::pair<int, int>>& terminals) {
  DSU dsu(nNodes);

  // Union sets for pairs defined in the input
  for (const auto& p : terminals) dsu.unite(p.first, p.second);

  // Group vertices by connected component (Root -> List of Nodes)
  std::map<int, std::vector<int>> groupsMap;
  std::vector<bool> isTerminal(nNodes, false);

  for (const auto& p : terminals) {
    isTerminal[p.first] = true;
    isTerminal[p.second] = true;
  }

  for (int i = 0; i < nNodes; ++i)
    if (isTerminal[i]) {
      int root = dsu.find(i);
      groupsMap[root].push_back(i);
    }

  // Convert map to vector of vectors
  std::vector<std::vector<int>> groupedTerminals;
  groupedTerminals.reserve(groupsMap.size());
  for (auto& entry : groupsMap)
    if (entry.second.size() > 1)
      groupedTerminals.push_back(std::move(entry.second));

  return groupedTerminals;
}

/**
 * @brief Generate Pairs (T)
 * Randomly pairs up terminals within each group until one remains.
 */
static std::vector<std::pair<int, int>> generatePairs(
    const std::vector<std::vector<int>>& terminalGroups) {
  std::vector<std::pair<int, int>> pairs;
  static std::random_device rd;
  static std::mt19937 rng(rd());

  for (const auto& groupConst : terminalGroups) {
    std::vector<int> group = groupConst;

    // While |terminalGroup| > 1
    while (group.size() > 1) {
      // Select random pivot
      std::uniform_int_distribution<int> distPivot(0, group.size() - 1);
      int idxPivot = distPivot(rng);
      int pivot = group[idxPivot];

      // Remove pivot (swap with back and pop)
      group[idxPivot] = group.back();
      group.pop_back();

      // Select random destination (from remaining)
      std::uniform_int_distribution<int> distDest(0, group.size() - 1);
      int idxDest = distDest(rng);
      int dest = group[idxDest];

      pairs.push_back({pivot, dest});
    }
  }

  return pairs;
}

/**
 * @brief Executes the GRASP Constructive Heuristic.
 */
SFPSolution GRASPConstructiveHeuristic::generate(
    const SFPProblem& problem) const {
  SFPSolution solution(problem);

  // Create a local copy of the graph (Working Graph) to modify weights
  // dynamically.
  Graph workingGraph = *problem.getGraphPtr();

  // Instantiate DijkstraEngine (Optimized reuse)
  DijkstraEngine dijkstra(problem.getNNodes());

  // Generate Pairs
  auto groups =
      preprocessTerminalGroups(problem.getNNodes(), problem.getTerminals());
  auto rawPairs = generatePairs(groups);

  // Initialize Candidate List
  std::vector<CandidatePair> CL;
  CL.reserve(rawPairs.size());
  for (const auto& pair : rawPairs)
    CL.push_back({pair.first, pair.second, std::numeric_limits<float>::max()});

  static std::random_device rd;
  static std::mt19937 rng(rd());

  // While |CL| >= 1
  while (!CL.empty()) {
    // Update costs in CL using Dijkstra on the workingGraph
    for (auto& cand : CL) {
      auto result =
          dijkstra.getShortPath(workingGraph, cand.source, cand.target);
      cand.currentPathCost = result.second;
    }

    // CL <- Sort(CL)
    std::sort(CL.begin(), CL.end());

    // RCL <- CL * alpha
    int rclSize = std::max(1, static_cast<int>(CL.size() * alpha));

    // Select random pair P from RCL
    std::uniform_int_distribution<int> distRCL(0, rclSize - 1);
    int selectedIdx = distRCL(rng);
    CandidatePair pair = CL[selectedIdx];

    // Connect Using Dijkstra
    auto result = dijkstra.getShortPath(workingGraph, pair.source, pair.target);
    const std::vector<int>& pathEdges = result.first;

    // Add path to solution and Zero costs
    const auto& originalEdges = problem.getGraphPtr()->edges;

    for (int edgeIdx : pathEdges) {
      // Update Solution using SFPMove
      if (!solution.isEdgeActive(edgeIdx)) {
        float originalWeight = originalEdges[edgeIdx].weight;
        SFPMove move(MoveType::ADD, edgeIdx, originalWeight);
        move.apply(solution);
      }

      // Zero the cost in the Working Graph
      // This encourages edge reuse for subsequent pairs
      workingGraph.edges[edgeIdx].weight = 0.0f;

      // Maintain bidirectional consistency
      int revIdx = workingGraph.edges[edgeIdx].reverseEdgePtr;
      if (revIdx != -1) workingGraph.edges[revIdx].weight = 0.0f;
    }

    // CL <- CL - {P}
    CL.erase(CL.begin() + selectedIdx);
  }

  return solution;
}
