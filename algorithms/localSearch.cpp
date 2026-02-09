#include "Solver.hpp"

/**
 * @brief Removes dead branches (non-terminal leaves) from the solution.
 * This ensures that all leaf nodes in the solution are Terminals.
 * @return true if any edge was removed.
 */
bool prune(SFPSolution& solution) {
  const auto& problem = solution.getProblem();
  const auto& graph = problem.getGraphPtr();
  const int nNodes = problem.getNNodes();
  const int nEdges = problem.getNEdges();

  // Calculate node degrees in the current solution
  std::vector<int> degree(nNodes, 0);
  bool changed = false;

  // We need to reconstruct degrees based on active edges
  for (int i = 0; i < nEdges; ++i)
    if (solution.isEdgeActive(i)) {
      const auto& e = graph->edges[i];
      // Consider only the canonical direction to count degree (undirected
      // graph)
      if (e.source < e.target) {
        degree[e.source]++;
        degree[e.target]++;
      }
    }

  // Identify Terminals (to avoid pruning)
  std::vector<bool> isTerminal(nNodes, false);
  for (const auto& pair : problem.getTerminals()) {
    isTerminal[pair.first] = true;
    isTerminal[pair.second] = true;
  }

  // Queue of candidate nodes for pruning (Degree 1 and Non-Terminal)
  std::queue<int> q;
  for (int i = 0; i < nNodes; ++i)
    if (degree[i] == 1 && !isTerminal[i]) q.push(i);

  // Cascading Pruning Loop
  while (!q.empty()) {
    int source = q.front();
    q.pop();

    // If the degree changed (e.g., became 0 because the neighbor disappeared
    // earlier), ignore
    if (degree[source] != 1) continue;

    // Find the active edge connected to u
    int edgeToRemove = -1;
    int target = -1;  // Neighbor

    // Simple linear search
    for (int i = graph->ptrs[source]; i < graph->ptrs[source + 1]; ++i) {
      if (solution.isEdgeActive(i)) {
        const auto& e = graph->edges[i];
        edgeToRemove = i;
        target = e.target;
        break;
      }
    }

    if (edgeToRemove != -1) {
      // Remove the edge
      SFPMove(MoveType::REMOVE, edgeToRemove, graph->edges[edgeToRemove].weight)
          .apply(solution);
      changed = true;

      // Update degrees
      degree[source]--;
      degree[target]--;

      // If the neighbor became a non-terminal leaf, add to queue to prune as
      // well
      if (degree[target] == 1 && !isTerminal[target]) q.push(target);
    }
  }
  return changed;
}

/**
 * @brief Implementation of the Local Search Phase.
 * * Strategy: "Destroy and Repair"
 * 1. Temporarily removes an edge from the current solution (Weight = INF).
 * 2. Identifies which terminal pairs became disconnected.
 * 3. Tries to reconnect them using the shortest path in the modified graph.
 * 4. If the reconstructed solution is cheaper, the move is accepted.
 */
bool GRASPLocalSearch::optimize(SFPSolution& solution) const {
  const auto& problem = solution.getProblem();

  // Create a local mutable copy of the graph to apply penalties (G' in the
  // paper) Using the copy constructor defined in Graph.hpp
  Graph workingGraph = *problem.getGraphPtr();

  // Reuse Dijkstra Engine for performance
  DijkstraEngine dijkstra(problem.getNNodes());

  // Constants
  const float INF = std::numeric_limits<float>::max();
  const int nEdges = problem.getNEdges();
  const int nNodes = problem.getNNodes();

  bool globalImprovement = false;
  // Initial pruning to clean up any mess from the constructive phase
  if (prune(solution)) globalImprovement = true;
  bool improvementFound = true;

  // Loop until no more improvements are found (Local Optimum)
  while (improvementFound) {
    improvementFound = false;

    // Identify all active edges in the current solution (S_P')
    std::vector<int> currentSolutionEdges;
    currentSolutionEdges.reserve(nEdges / 2);
    for (int i = 0; i < nEdges; ++i)
      if (solution.isEdgeActive(i)) {
        const auto& edge = workingGraph.edges[i];
        // Only process the edge if source < target
        // This ensures we don't try to remove 0->1 AND 1->0 separately.
        if (edge.source < edge.target) currentSolutionEdges.push_back(i);
      }

    // Iterate over each edge
    for (int edgeToRemove : currentSolutionEdges) {
      // Set cost of the current edge to infinity
      float originalWeight = workingGraph.edges[edgeToRemove].weight;
      workingGraph.edges[edgeToRemove].weight = INF;

      // Must also update the reverse edge to maintain graph consistency
      int revIdx = workingGraph.edges[edgeToRemove].reverseEdgePtr;
      if (revIdx != -1) workingGraph.edges[revIdx].weight = INF;

      // Efficiently identify broken connectivity using a temporary DSU
      DSU solDSU(nNodes);

      // Rebuild components using ALL active edges EXCEPT the one we removed
      for (int i = 0; i < nEdges; ++i)
        if (solution.isEdgeActive(i))
          // Skip the removed edge and its reverse
          if (i != edgeToRemove && i != revIdx) {
            const auto& e = workingGraph.edges[i];
            // Union only once per physical edge
            if (e.source < e.target) solDSU.unite(e.source, e.target);
          }

      // Create a candidate solution based on the current one
      SFPSolution candidate = solution;

      // Remove the edge 'e' from the candidate solution
      SFPMove removeMove(MoveType::REMOVE, edgeToRemove, originalWeight);
      removeMove.apply(candidate);

      // For each (pivot, destination) affected...
      bool reconstructionPossible = true;
      for (const auto& pair : problem.getTerminals()) {
        int source = pair.first;
        int target = pair.second;

        // Check if this specific pair was disconnected
        if (!solDSU.isConnected(source, target)) {
          // Reconnect using Dijkstra avoiding infinite edges
          auto result = dijkstra.getShortPath(workingGraph, source, target);
          float pathCost = result.second;
          const std::vector<int>& newPath = result.first;

          // If path cost is infinite or negative, the graph became disconnected
          if (pathCost >= INF || pathCost < 0) {
            reconstructionPossible = false;
            break;
          }

          // Update pivot and destination with the new path
          for (int newEdgeIdx : newPath)
            if (!candidate.isEdgeActive(newEdgeIdx)) {
              // Get the real weight from the graph
              float weight = workingGraph.edges[newEdgeIdx].weight;
              SFPMove addMove(MoveType::ADD, newEdgeIdx, weight);
              addMove.apply(candidate);

              // Update local DSU to avoid re-routing if multiple
              // pairs share the same missing link that was just fixed.
              const auto& e = workingGraph.edges[newEdgeIdx];
              solDSU.unite(e.source, e.target);
            }
        }
      }

      // If newCost < cost then
      if (reconstructionPossible &&
          candidate.getObjectiveValue() < solution.getObjectiveValue()) {
        // Update (Sp, Sv)
        solution = candidate;

        globalImprovement = true;
        improvementFound = true;

        // Restore the removed edge
        workingGraph.edges[edgeToRemove].weight = originalWeight;
        if (revIdx != -1) workingGraph.edges[revIdx].weight = originalWeight;

        break;
      }

      // We just restore the graph weight. The 'candidate' object is discarded.
      workingGraph.edges[edgeToRemove].weight = originalWeight;
      if (revIdx != -1) workingGraph.edges[revIdx].weight = originalWeight;
    }
  }

  // Final pruning: Ensures the final solution has no loose ends
  // This is very important as the "Reconnect" process can create new dead
  // branches
  if (prune(solution)) globalImprovement = true;

  return globalImprovement;
}
