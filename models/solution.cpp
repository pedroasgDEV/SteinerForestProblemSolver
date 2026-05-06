#include "../utils/DSU.hpp"
#include "SFP.hpp"

SFPSolution::SFPSolution(const SFPProblem* problem,
                         std::vector<SolutionPair> init_pairs)
    : problem(problem),
      edges(std::vector<int>(problem->getNEdges(), -1)),
      bitmask(std::vector<uint8_t>(problem->getNEdges(), 0)),
      nodes(std::vector<std::pair<uint8_t, int>>(problem->getNNodes(), {0, -1})),
      currentCost(0.0f) {
  
  active_edges.reserve(problem->getNNodes());
  pairs.reserve(problem->getNTerminals());
  
  if (!init_pairs.empty()) {
    pairs = std::move(init_pairs);
    for (const auto& p : pairs) {
      nodes[p.source].first = 1;
      nodes[p.target].first = 1;
    }
  } else {
    for (const auto& p : problem->getTerminals()) {
      pairs.push_back({p.first, p.second, problem->getNTerminals()});
      nodes[p.first].first = 1;
      nodes[p.second].first = 1;
    }
  }
}


bool SFPSolution::isFeasible() const {
  const auto& graph = problem->getGraphPtr();
  DSU dsu(graph->nNodes);
  const auto& graphEdges = graph->edges;

  for (const auto& i : active_edges) {
    const auto& edge = graphEdges[i.id];
    dsu.unite(edge.source, edge.target);
  }

  const auto& terminals = problem->getTerminals();
  for (const auto& pair : terminals)
    if (!dsu.isConnected(pair.first, pair.second)) return false;
  return true;
}

int SFPSolution::insert(const int edge_id, const int pair_id) {
  if (edge_id < 0 || edge_id >= problem->getNEdges()) return 0;
  if (pair_id < 0 || pair_id >= (int)pairs.size()) return 0;

  auto graph_edge = problem->getGraphPtr()->edges[edge_id];
  auto reverse_id = graph_edge.reverseEdgePtr;

  int active_idx = edges[edge_id];

  if (active_idx == -1) {
    active_idx = active_edges.size();
    active_edges.emplace_back(edge_id, reverse_id, graph_edge.weight);
    
    edges[edge_id] = active_idx;
    bitmask[edge_id] = 1;
    if (reverse_id != -1) {
      edges[reverse_id] = active_idx;
      bitmask[reverse_id] = active_idx;
    }

    currentCost += graph_edge.weight;
  }

  auto& pair_edges = pairs[pair_id].edges;
  
  if (std::find(pair_edges.begin(), pair_edges.end(), edge_id) == pair_edges.end()) {
    pair_edges.push_back(edge_id);
    pairs[pair_id].pathCost += graph_edge.weight;

    auto& edge_pairs = active_edges[active_idx].pairs;

    for (int otrPair : edge_pairs) {
      pairs[pair_id].competing_pairs_sum[otrPair]++;
      pairs[pair_id].synergy++;
      pairs[otrPair].competing_pairs_sum[pair_id]++;
      pairs[otrPair].synergy++;
    }

    edge_pairs.push_back(pair_id);
  }

  return 1;
}

int SFPSolution::erase(const int edge_id, const int pair_id) {
  if (edge_id < 0 || edge_id >= problem->getNEdges()) return 0;
  if (pair_id < 0 || pair_id >= (int)pairs.size()) return 0;

  if (edges[edge_id] == -1) return 0;

  int active_idx = edges[edge_id];
  auto& edge_obj = active_edges[active_idx];
  auto& edge_pairs = edge_obj.pairs;

  auto& pair_edges = pairs[pair_id].edges;
  auto it_pe = std::find(pair_edges.begin(), pair_edges.end(), edge_id);
  
  if (it_pe != pair_edges.end()) {
    *it_pe = pair_edges.back();
    pair_edges.pop_back();

    pairs[pair_id].pathCost -= edge_obj.weight;

    for (int otrPair : edge_pairs) {
      if (otrPair != pair_id && pairs[pair_id].competing_pairs_sum[otrPair] > 0) {
        pairs[pair_id].competing_pairs_sum[otrPair]--;
        pairs[pair_id].synergy--;
        pairs[otrPair].competing_pairs_sum[pair_id]--;
        pairs[otrPair].synergy--;
      }
    }

    auto it_ep = std::find(edge_pairs.begin(), edge_pairs.end(), pair_id);
    if (it_ep != edge_pairs.end()) {
      *it_ep = edge_pairs.back();
      edge_pairs.pop_back();
    }

    if (edge_pairs.empty()) {
      currentCost -= edge_obj.weight;

      int last_idx = active_edges.size() - 1;
      
      if (active_idx != last_idx) {
        active_edges[active_idx] = active_edges.back();
        
        edges[active_edges[active_idx].id] = active_idx;
        if (active_edges[active_idx].reverse_id != -1) {
          edges[active_edges[active_idx].reverse_id] = active_idx;
        }
      }
      active_edges.pop_back();

      auto reverse_id = problem->getGraphPtr()->edges[edge_id].reverseEdgePtr;
      edges[edge_id] = -1;
      bitmask[edge_id] = 0;
      if (reverse_id != -1) {
        edges[reverse_id] = -1;
        bitmask[reverse_id] = 0;
      }
    }
    return 1;
  }
  return 0;
}

std::ostream& operator<<(std::ostream& out, const SFPSolution& sol) {
  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  out << "Solution Cost: " << sol.currentCost << std::endl;
  out << "Active Edges: [";
  const auto& graphEdges = sol.problem->getGraphPtr()->edges;

  if (sol.edges.empty()) out << " None ";
  for (const auto& i : sol.active_edges)
    out << " (" << graphEdges[i.id].source << "->" << graphEdges[i.id].target
        << ") ";

  out << "]";

  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  return out;
}
