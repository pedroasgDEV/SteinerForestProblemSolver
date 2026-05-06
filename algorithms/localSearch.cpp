#include <algorithm>
#include <utility>
#include <vector>

#include "Solver.hpp"

bool GRASPLocalSearch::optimize(SFPSolution* solution) {
  if (!dijkstra)
    dijkstra = std::make_shared<BidirectionalDijkstraEngine>(
        solution->getProblem()->getGraphPtr());

  int nEdges = solution->getProblem()->getNEdges();
  std::vector<uint8_t> ditchs(nEdges, 0);

  const std::vector<SolutionEdge>* edgesToTest = solution->getEdges();

  std::vector<int> affectedPairs;
  affectedPairs.reserve(solution->getNPairs());
  SFPNeighborhood destroy;
  destroy.moves.reserve(solution->getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(solution->getNPairs());

  double currentBestCost = solution->getCurrentCost();
  bool foundAnyImprovement = false;

  for (const auto& edgeToDrop : *edgesToTest) {
    if (!solution->isEdgeActive(edgeToDrop.id)) continue;

    ditchs[edgeToDrop.id] = 1;
    if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 1;

    affectedPairs.clear();
    destroy.moves.clear();
    repair.moves.clear();

    for (int pair_id : *solution->getEdgePairs(edgeToDrop.id)){
      affectedPairs.push_back(pair_id);
      destroy.moves.push_back({solution, MoveType::DSCNCT_PAIR, pair_id, *solution->getPairEdges(pair_id)});
    }

    destroy.apply();
    bool feasible = true;

    for (auto pair : affectedPairs) {
      auto [source, target] = solution->getPairNodes(pair);

      auto result = dijkstra->getShortPath(source, target, solution->getBitmask(), &ditchs);

      if (result.second < 0) {
        feasible = false;
        break;
      }

      repair.addMoveApplying({solution, MoveType::CNCT_PAIR, pair, std::move(result.first)});
    }

    double newCost = solution->getCurrentCost();

    if (feasible && newCost < currentBestCost - 1e-4) {
      currentBestCost = newCost;
      foundAnyImprovement = true;
    } else {
      repair.undo();
      destroy.undo();
    }

    ditchs[edgeToDrop.id] = 0;
    if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 0;
  }
  return foundAnyImprovement;
}


bool HubBreakingLocalSearch::optimize(SFPSolution* solution) {
  if (!dijkstra)
    dijkstra = std::make_shared<BidirectionalDijkstraEngine>(
        solution->getProblem()->getGraphPtr());

  const auto* problem = solution->getProblem();
  const auto graph = problem->getGraphPtr();
  int nNodes = problem->getNNodes();
  int nEdges = problem->getNEdges();

  struct HubInfo {
    int nodeId;
    std::vector<int> activeEdges;
    double totalCost = 0.0;
    double instabilityIndex = 0.0;
  };

  std::vector<HubInfo> hubs(nNodes);
  for (int i = 0; i < nNodes; ++i) hubs[i].nodeId = i;

  const auto* active_edges = solution->getEdges();
  for (const auto& sEdge : *active_edges) { 
    int u = graph->edges[sEdge.id].source;
    int v = graph->edges[sEdge.id].target;
    double weight = sEdge.weight; 

    hubs[u].activeEdges.push_back(sEdge.id);
    hubs[u].totalCost += weight;

    hubs[v].activeEdges.push_back(sEdge.id);
    hubs[v].totalCost += weight;
  }

  std::vector<HubInfo> hubsToTest;
  hubsToTest.reserve(nNodes);

  for (auto& hub : hubs) {
    if ((int) hub.activeEdges.size() < 3) continue;
 
    std::vector<int> affectedPairs; 
    for (int edgeId : hub.activeEdges) { 
      const auto* pList = solution->getEdgePairs(edgeId); 
      affectedPairs.insert(affectedPairs.end(), pList->begin(), pList->end());
    }
    
    std::sort(affectedPairs.begin(), affectedPairs.end());
    affectedPairs.erase(std::unique(affectedPairs.begin(), affectedPairs.end()), affectedPairs.end());

    if (affectedPairs.empty()) continue;
 
    double sumSynergy = 0.0; 
    for (int pId : affectedPairs) 
      sumSynergy += (1.0 + solution->getPair(pId).synergy); 
    
    double averageSynergy = sumSynergy / affectedPairs.size();

    hub.instabilityIndex = hub.totalCost / averageSynergy;
    hubsToTest.push_back(hub);
  }

  std::sort(hubsToTest.begin(), hubsToTest.end(),
            [](const HubInfo& a, const HubInfo& b) {
              return a.instabilityIndex > b.instabilityIndex;
            }); 
 
  std::vector<uint8_t> ditchs(nEdges, 0); 
  double currentBestCost = solution->getCurrentCost(); 
  bool foundAnyImprovement = false;

  for (const auto& hub : hubsToTest) {
 
    std::vector<int> validEdgesToDrop; 
    for (int edgeId : hub.activeEdges) 
      if (solution->isEdgeActive(edgeId)) validEdgesToDrop.push_back(edgeId);

    if ((int) validEdgesToDrop.size() < 3) continue;
 
    std::vector<int> pairsToRepair; 
    for (int edgeId : validEdgesToDrop) { 
      const auto* pList = solution->getEdgePairs(edgeId); 
      pairsToRepair.insert(pairsToRepair.end(), pList->begin(), pList->end());
    }
    std::sort(pairsToRepair.begin(), pairsToRepair.end());
    pairsToRepair.erase(std::unique(pairsToRepair.begin(), pairsToRepair.end()), pairsToRepair.end());

    std::sort(pairsToRepair.begin(), pairsToRepair.end(), [&](int a, int b) {
      return solution->getPair(a).synergy > solution->getPair(b).synergy; 
    });

    for (int id : validEdgesToDrop) {
      ditchs[id] = 1;
      int rev = graph->edges[id].reverseEdgePtr;
      if (rev != -1) ditchs[rev] = 1;
    }  
  
    SFPNeighborhood destroy; 
    SFPNeighborhood repair;

    for (int pId : pairsToRepair) {  
      std::vector<int> currentPath = *solution->getPairEdges(pId);  
      destroy.addMoveApplying({solution, MoveType::DSCNCT_PAIR, pId, currentPath}); 
    }
 
    bool feasible = true; 
    for (int pId : pairsToRepair) {
      auto [src, tgt] = solution->getPairNodes(pId);  
       
      auto res = dijkstra->getShortPath(src, tgt, solution->getBitmask(), &ditchs); 

      if (res.second < 0) {
        feasible = false; 
        break; 
      } 
      repair.addMoveApplying({solution, MoveType::CNCT_PAIR, pId, std::move(res.first)});
    } 
 
    if (feasible && solution->getCurrentCost() < currentBestCost - 1e-4) {
      currentBestCost = solution->getCurrentCost();
      foundAnyImprovement = true;
    } 
    else {  
      repair.undo();  
      destroy.undo(); 
    }

    for (int id : validEdgesToDrop) {
      ditchs[id] = 0;
      int rev = graph->edges[id].reverseEdgePtr;
      if (rev != -1) ditchs[rev] = 0;
    }
  }

  return foundAnyImprovement;
}
