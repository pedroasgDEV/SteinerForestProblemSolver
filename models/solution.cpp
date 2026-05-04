#include "SFP.hpp"
#include "../utils/DSU.hpp"

SFPSolution::SFPSolution(const SFPProblem* problem, std::vector<SolutionPair> init_pairs)
    : problem(problem), bitmask(std::vector<uint8_t>(problem->getNEdges(), 0)), currentCost(0.0f) { 
  edges.reserve(problem->getNNodes());
  pairs.reserve(problem->getNTerminals());
  if (!init_pairs.empty()) pairs = std::move(init_pairs);
  else
    for (const auto& p : problem->getTerminals())
      pairs.push_back({p.first, p.second, problem->getNTerminals()});
}

bool SFPSolution::isEdgeActive( const int edge_id ) const{
  if(edge_id < 0 || edge_id >= problem->getNEdges()) return false;
  return bitmask[edge_id];
}

std::vector<int> SFPSolution::getEdgePairs(const SolutionEdge& edge) const{
  auto it = edges.find(edge);
  if (it == edges.end()) return {};
  std::vector<int> result;
  result.reserve(it->pairs.size());
  for(int e : it->pairs) result.push_back(e);
  return result;
}

std::vector<int> SFPSolution::getPairEdges(const int pair_id) const{
  if(pair_id < 0 || pair_id >= (int) pairs.size()) return {};
  std::vector<int> result;
  result.reserve(pairs[pair_id].edges.size());
  for(int e : pairs[pair_id].edges) result.push_back(e);
  return result;
}

double SFPSolution::getPairCost(const int pair_id) const{
  if(pair_id < 0 || pair_id >= (int) pairs.size()) return -1.0f; 
  return pairs[pair_id].pathCost;
}

bool SFPSolution::isFeasible() const{
  const auto& graph = problem->getGraphPtr();
  DSU dsu(graph->nNodes);
  const auto& graphEdges = graph->edges;

  for (const auto& i : edges) {
    const auto& edge = graphEdges[i.id];
    dsu.unite(edge.source, edge.target);
  }

  const auto& terminals = problem->getTerminals();
  for (const auto& pair : terminals)
    if (!dsu.isConnected(pair.first, pair.second)) return false;
  return true;
}

int SFPSolution::insert(const int edge_id, const int pair_id) {
  if(edge_id < 0 || edge_id >= problem->getNEdges()) return 0;
  if(pair_id < 0 || pair_id >= (int) pairs.size()) return 0;
  
  auto edge = problem->getGraphPtr()->edges[edge_id];
  auto reverse_id = edge.reverseEdgePtr;

  auto [it, inserted] = edges.insert({edge_id, reverse_id, edge.weight});
  if(it == edges.end()) return 0;

  if(pairs[pair_id].edges.insert(it->id).second){ 
    pairs[pair_id].pathCost += it->weight;
    
    for(auto otrPair : it->pairs){
      pairs[pair_id].competing_pairs_sum[otrPair]++;
      pairs[otrPair].competing_pairs_sum[pair_id]++;
    }

    it->pairs.insert(pair_id);
  }

  bitmask[it->id] = 1;
  if(reverse_id != -1) bitmask[it->reverse_id] = 1;
  
  if (inserted) currentCost += edge.weight;
  return 1;
}

int SFPSolution::erase(const int edge_id, const int pair_id) {
  if(edge_id < 0 || edge_id >= problem->getNEdges()) return 0;
  if(pair_id < 0 || pair_id >= (int) pairs.size() ) return 0;

  auto edge = problem->getGraphPtr()->edges[edge_id];
  auto reverse_id = edge.reverseEdgePtr;
  auto edge_ptr = edges.find({edge_id, reverse_id, edge.weight});
  if(edge_ptr == edges.end()) return 0;
  
  if(pairs[pair_id].edges.erase(edge_ptr->id)){
    pairs[pair_id].pathCost -= edge_ptr->weight;

    for(auto otrPair : edge_ptr->pairs)
      if(otrPair != pair_id && pairs[pair_id].competing_pairs_sum[otrPair] > 0){
         pairs[pair_id].competing_pairs_sum[otrPair]--;
         pairs[otrPair].competing_pairs_sum[pair_id]--;
      } 

    edge_ptr->pairs.erase(pair_id);
    if(!edge_ptr->pairs.empty()) return 1;
  } 
  else return 0;

  bitmask[edge_ptr->id] = 0;
  if(edge_ptr->reverse_id != -1) bitmask[edge_ptr->reverse_id] = 0;
  
  if(!edges.erase(*edge_ptr)) return 0;
  
  currentCost -= edge.weight;
  return 1;
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
  const auto& graphEdges = sol.problem->getGraphPtr()->edges;
  
  if(sol.edges.empty()) out << " None ";
  for (const auto& i : sol.edges) 
    out << " (" << graphEdges[i.id].source << "->" << graphEdges[i.id].target << ") ";

  out << "]";

  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  return out;
}
