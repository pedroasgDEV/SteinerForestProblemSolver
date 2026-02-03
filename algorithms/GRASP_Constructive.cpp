#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include "algorithms.hpp"

struct TerminalPair {
  int u;
  int v;
  float cost;
};

static std::vector<std::pair<int, int>> generatePairs(
    const std::vector<std::set<int>>& terminals) {
  std::vector<std::pair<int, int>> pairs;

  std::random_device rd;
  std::mt19937 gen(rd());

  for (auto group : terminals)
    while (group.size() > 1) {
      // Select a random pivot from the group
      std::vector<int> tempGroup(group.begin(), group.end());
      std::uniform_int_distribution<> dis(0, tempGroup.size() - 1);
      int pivotIndex = dis(gen);
      int pivot = tempGroup[pivotIndex];

      // Remove the pivot
      group.erase(pivot);
      tempGroup.erase(tempGroup.begin() + pivotIndex);

      // Select another node for the destination
      std::uniform_int_distribution<> disDest(0, tempGroup.size() - 1);
      int dest = tempGroup[disDest(gen)];

      // Add to pairs vector
      pairs.push_back({pivot, dest});
    }

  return pairs;
}

// Merging intersecting sets
static std::vector<std::set<int>> preprocessTerminals(
    int nNodes, const std::vector<std::pair<int, int>>& terminals) {
  std::vector<int> parent(nNodes);
  for (int i = 0; i < nNodes; i++) parent[i] = i;

  // Find root
  auto findSet = [&](int i) {
    while (i != parent[i]) {
      parent[i] = parent[parent[i]];
      i = parent[i];
    }
    return i;
  };

  // Merge sets
  auto unionSets = [&](int i, int j) {
    int rootI = findSet(i);
    int rootJ = findSet(j);
    if (rootI != rootJ) parent[rootI] = rootJ;
  };

  // Merge based on terminal pairs
  for (auto p : terminals) unionSets(p.first, p.second);

  // Group into sets
  std::map<int, std::set<int>> groupsMap;
  for (auto p : terminals) {
    int root = findSet(p.first);
    groupsMap[root].insert(p.first);
    groupsMap[root].insert(p.second);
  }

  // Group only by node IDs
  std::vector<std::set<int>> groups;
  for (auto const& entry : groupsMap) groups.push_back(entry.second);

  return groups;
}

/**
 * @brief GRASP Constructive Heuristic.
 * @param originalGraph SFP initial graph.
 * @param terminals Pairs of terminals that need to be connected.
 * @param alpha RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
 * @return Graph with the SFP solution.
 */
Graph GRASPconstructiveHeuristic(
    const Graph& originalGraph,
    const std::vector<std::pair<int, int>>& terminals, float alpha) {
  Graph solution(originalGraph.getNNode(), originalGraph.getIsBidirectional());
  Graph workingGraph = originalGraph;

  // Agglutination and Selection of Pairs
  auto terminalGroups = preprocessTerminals(workingGraph.getNNode(), terminals);
  auto pairsRaw = generatePairs(terminalGroups);

  // Convert to struct to handle costs and get minimum cost using dijkstra
  std::vector<TerminalPair> CL;
  CL.reserve(pairsRaw.size());
  for (auto p : pairsRaw)
    CL.push_back(
        {p.first, p.second, dijkstra(workingGraph, p.first, p.second).second});

  std::random_device rd;
  std::mt19937 gen(rd());

  while (!CL.empty()) {
    // Sort CL
    std::sort(CL.begin(), CL.end(),
              [](const TerminalPair& a, const TerminalPair& b) {
                return a.cost < b.cost;
              });

    // Create RCL = CL * alpha
    int rclSize = std::max(1, static_cast<int>(CL.size() * alpha));

    // Select random pair P from RCL
    std::uniform_int_distribution<> dis(0, rclSize - 1);
    int idx = dis(gen);
    TerminalPair selectedP = CL[idx];

    // Connect using Dijkstra
    std::vector<int> path =
        dijkstra(workingGraph, selectedP.u, selectedP.v).first;

    // Update Solution
    auto matrixOrig = originalGraph.getMatrix();
    for (size_t i = 0; i < path.size() - 1; i++) {
      int u = path[i];
      int v = path[i + 1];

      // Add edge to solution
      if (!solution.isAdjacent(u, v)) solution.addEdge(u, v, matrixOrig[u][v]);

      // Almost zero the weight of edges ensuring next pairs use them
      workingGraph.addEdge(u, v, 0.01f);
    }

    // CL <- CL - {P}
    CL.erase(CL.begin() + idx);
  }

  return solution;
}
