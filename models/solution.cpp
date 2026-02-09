#include "SFP.hpp"

// SFPMove
void SFPMove::apply(SFPSolution& sol) const {
  if (type == MoveType::ADD) {
    sol.internalAdd(edgeIndex);
    sol.currentCost += costDelta;
  } else {
    sol.internalRemove(edgeIndex);
    sol.currentCost -= costDelta; 
  }
}

void SFPMove::undo(SFPSolution& sol) const {
  if (type == MoveType::ADD) {
    sol.internalRemove(edgeIndex);
    sol.currentCost -= costDelta; 
  } else {
    sol.internalAdd(edgeIndex);
    sol.currentCost += costDelta; 
  }
}

// SFPSolution
SFPSolution::SFPSolution(const SFPProblem& prob)
    : problem(&prob), currentCost(0.0f) {
  activeEdges.resize(problem->getNEdges(), false);
}

void SFPSolution::internalAdd(int edgeIdx) {
  if (!activeEdges[edgeIdx]) {
    activeEdges[edgeIdx] = true;
    int revIdx = problem->getGraphPtr()->edges[edgeIdx].reverseEdgePtr;
    if (revIdx != -1) activeEdges[revIdx] = true;
  }
}

void SFPSolution::internalRemove(int edgeIdx) {
  if (activeEdges[edgeIdx]) {
    activeEdges[edgeIdx] = false;
    int revIdx = problem->getGraphPtr()->edges[edgeIdx].reverseEdgePtr;
    if (revIdx != -1) activeEdges[revIdx] = false;
  }
}

bool SFPSolution::isFeasible(DSU& dsu) const {
  dsu.reset();

  const auto& graph = problem->getGraphPtr();
  const auto& edges = graph->edges;

  for (size_t i = 0; i < activeEdges.size(); ++i) {
    if (activeEdges[i]) {
      const auto& edge = edges[i];
      // Since DSU is undirected, processing u->v is enough.
      // This cuts DSU operations by 50%.
      if (edge.source < edge.target) dsu.unite(edge.source, edge.target);
    }
  }

  const auto& terminals = problem->getTerminals();
  for (const auto& pair : terminals)
    if (!dsu.isConnected(pair.first, pair.second)) return false;
  return true;
}

bool SFPSolution::operator>(const SFPSolution& other) const {
  return currentCost > other.currentCost;
}
bool SFPSolution::operator<(const SFPSolution& other) const {
  return currentCost < other.currentCost;
}

std::ostream& operator<<(std::ostream& out, const SFPSolution& sol) {
  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  out << "Solution Cost: " << sol.currentCost << std::endl;
  out << "Active Edges: [";
  const auto& edges = sol.problem->getGraphPtr()->edges;

  int count = 0;
  for (size_t i = 0; i < sol.activeEdges.size(); i++)
    if (sol.activeEdges[i])
      if (edges[i].source < edges[i].target) {
        out << " (" << edges[i].source << "->" << edges[i].target << ") ";
        count++;
      }

  if (count == 0) out << " None ";
  out << "]";

  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  return out;
}
