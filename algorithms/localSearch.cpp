#include "Solver.hpp"

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

  return globalImprovement;
}
