#include "Solver.hpp"

#include "../utils/DSU.hpp"
#include <unordered_map>

/**
 * @brief Agglutinates terminal sets that share vertices.
 */
static std::vector<std::vector<int>> preprocessTerminalGroups(
    int nNodes, const std::vector<std::pair<int, int>>& terminals) {
  DSU dsu(nNodes);

  // Union sets for pairs defined in the input
  for (const auto& p : terminals) dsu.unite(p.first, p.second);

  // Group vertices by connected component (Root -> List of Nodes)
  std::unordered_map<int, std::vector<int>> groupsMap;
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
static std::vector<SolutionPair> generatePairs(
    const std::vector<std::vector<int>>& terminalGroups, const int nTerm, std::mt19937& rng) {
  
  std::vector<SolutionPair> pairs;

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

      pairs.push_back({pivot, dest, nTerm});
    }
  }

  return pairs;
}

/**
 * @struct Candidate
 * @brief Represents an element in the Candidate List (CL)
 */
struct Candidate {
  int pair_id;
  double cost;
  std::vector<int> path;

  Candidate(const int pair_id) : pair_id(pair_id), cost(0.0) {}

  bool operator<(const Candidate& other) const { return cost < other.cost; }
};

/**
 * @brief Executes the GRASP Constructive Heuristic.
 */
SFPSolution GRASPConstructiveHeuristic::generate(const SFPProblem* problem, std::mt19937& rng) {

  if(!dijkstra) dijkstra = std::make_shared<DijkstraEngine>(problem->getGraphPtr());
    
  // Generate Pairs
  auto groups = preprocessTerminalGroups(problem->getNNodes(), problem->getTerminals());
  auto rawPairs = generatePairs(groups, problem->getTerminals().size(), rng);

  std::vector<SolutionPair> dictPairs = rawPairs;
  SFPSolution solution(problem, std::move(rawPairs));

  // Initialize Candidate List (CL)
  std::vector<Candidate> CL; CL.reserve(dictPairs.size());

  for (int i = 0; i < static_cast<int>(dictPairs.size()); ++i) {
    Candidate cand(i);
    auto result = dijkstra->getShortPath(dictPairs[i].source, dictPairs[i].target, solution.getBitmask());
    cand.path = std::move(result.first);
    cand.cost = result.second;
    CL.push_back(std::move(cand));
  }

  // While |CL| > 0
  while (!CL.empty()) {

    // CL <- Sort(CL)
    std::sort(CL.begin(), CL.end());

    // RCL <- CL * alpha
    int rclSize = std::max(1, static_cast<int>(CL.size() * alpha));

    // Select random candidate from RCL
    std::uniform_int_distribution<int> distRCL(0, rclSize - 1);
    int selectedIdx = distRCL(rng);

    // Apply the connection using our safe ConnectPairMove
    SFPMove move(&solution, MoveType::CNCT_PAIR, CL[selectedIdx].pair_id, std::move(CL[selectedIdx].path));
    move.apply();

    // CL <- CL - {P} (O(1) removal technique)
    CL[selectedIdx] = std::move(CL.back());
    CL.pop_back();

    // Update costs and paths in CL using Dijkstra
    for (auto& cand : CL) {
      auto result = dijkstra->getShortPath(dictPairs[cand.pair_id].source, dictPairs[cand.pair_id].target, solution.getBitmask());
      cand.path = std::move(result.first);
      cand.cost = result.second;
    }
  }

  return solution;
}

SFPSolution SimpleConstructiveHeuristic::generate(const SFPProblem* problem, std::mt19937& rng) {

  if(!dijkstra) dijkstra = std::make_shared<DijkstraEngine>(problem->getGraphPtr());
    
  // Generate Pairs
  auto groups = preprocessTerminalGroups(problem->getNNodes(), problem->getTerminals());
  auto rawPairs = generatePairs(groups, problem->getTerminals().size(), rng);
  
  std::vector<SolutionPair> dictPairs = rawPairs;
  SFPSolution solution(problem, std::move(rawPairs));

  for (int i = 0; i < static_cast<int>(dictPairs.size()); ++i) {
    // Find a path to connect the pair;
    auto result = dijkstra->getShortPath(dictPairs[i].source, dictPairs[i].target, solution.getBitmask());
    
    // Add the path to the solution
    SFPMove move(&solution, MoveType::CNCT_PAIR, i, std::move(result.first));
    move.apply();
  }

  return solution;
}
