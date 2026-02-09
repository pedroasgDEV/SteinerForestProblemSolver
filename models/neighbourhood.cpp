#include "SFP.hpp"

std::vector<SFPMove> AddNeighbourhood::moves(const SFPSolution& sol) {
  std::vector<SFPMove> moveList;
  const auto& graph = problem.getGraphPtr();
  const auto& edges = graph->edges;

  // Memory reservation optimization. "/4" is an estimated number.
  moveList.reserve(graph->nEdges / 4);

  for (int i = 0; i < graph->nEdges; ++i)
    if (!sol.isEdgeActive(i)) {
      const auto& edge = edges[i];
      // Prevents generating duplicate moves for (u->v) and (v->u).
      if (edge.source < edge.target)
        moveList.emplace_back(MoveType::ADD, i, edge.weight);
    }
  return moveList;
}

std::vector<SFPMove> RemoveNeighbourhood::moves(const SFPSolution& sol) {
  std::vector<SFPMove> moveList;
  const auto& graph = problem.getGraphPtr();
  const auto& edges = graph->edges;

  // Memory reservation optimization "/4" is an estimated number.
  moveList.reserve(graph->nEdges / 4);

  for (int i = 0; i < graph->nEdges; ++i)
    if (sol.isEdgeActive(i)) {
      const auto& edge = edges[i];
      // Prevents generating duplicate moves for (u->v) and (v->u).
      if (edge.source < edge.target)
        moveList.emplace_back(MoveType::REMOVE, i, -edge.weight);
    }
  return moveList;
}
