#include <algorithm>
#include <vector>
#include "Solver.hpp"

/**
 * @brief Implementation of the Local Search Phase for GRASP.
 * * Strategy: "Destroy and Repair"
 * 1. Temporarily removes an edge from the current solution (Weight = INF).
 * 2. Identifies which terminal pairs became disconnected (including competitors).
 * 3. Tries to reconnect them using the shortest path in the modified graph.
 * 4. If the reconstructed solution is cheaper, the move is accepted.
 */
bool GRASPLocalSearch::optimize(SFPSolution* solution, std::mt19937& rng) {
  if (!dijkstra)
    dijkstra = std::make_shared<DijkstraEngine>(solution->getProblem()->getGraphPtr());

  int nEdges = solution->getProblem()->getNEdges();
  std::vector<uint8_t> ditchs(nEdges, 0);
  
  std::vector<SolutionEdge> edgesToTest = solution->getEdges();
  std::shuffle(edgesToTest.begin(), edgesToTest.end(), rng);

  std::unordered_set<int> affectedPairs;
  affectedPairs.reserve(solution->getNPairs());
  SFPNeighborhood destroy;
  destroy.moves.reserve(solution->getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(solution->getNPairs());
  std::vector<int> pairsToRepair;
  pairsToRepair.reserve(solution->getNPairs());

  for (const auto& edgeToDrop : edgesToTest) {
    if (!solution->isEdgeActive(edgeToDrop.id)) continue;

    ditchs[edgeToDrop.id] = 1;
    if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 1;

    double originalCost = solution->getCurrentCost();

    affectedPairs.clear();
    destroy.moves.clear();
    pairsToRepair.clear();
    repair.moves.clear();

    // LEVEL 1 (The Epicenter): Pairs that pass exactly through the removed edge
    std::vector<int> currentWave;
    for (int pair_id : solution->getEdgePairs(edgeToDrop))
      if (affectedPairs.insert(pair_id).second) {
        destroy.moves.push_back({solution, MoveType::DSCNCT_PAIR, pair_id, solution->getPairEdges(pair_id)});
        currentWave.push_back(pair_id);
      }

    // LEVELS 2 TO DELTA (The Shock Wave): Cascading Competitors
    for (int d = 2; d <= delta; ++d) {
      std::vector<int> nextWave;
      for (int p : currentWave)
        for (int comp : solution->getCompetingPairs(p))
          if (affectedPairs.insert(comp).second) {
            destroy.moves.push_back({solution, MoveType::DSCNCT_PAIR, comp, solution->getPairEdges(comp)});
            nextWave.push_back(comp);
          }
      if (nextWave.empty()) break; 
      currentWave = std::move(nextWave);
    }

    destroy.apply();
    bool feasible = true;
    pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
    std::shuffle(pairsToRepair.begin(), pairsToRepair.end(), rng);
    
    for (auto pair : pairsToRepair) {
      auto [source, target] = solution->getPairNodes(pair);
      auto result = dijkstra->getShortPath(source, target, solution->getBitmask(), &ditchs);
      
      if (result.second < 0) {
        feasible = false;
        break;
      }
      repair.addMoveApplying({solution, MoveType::CNCT_PAIR, pair, std::move(result.first)});
    }

    double newCost = solution->getCurrentCost();
    
    if (feasible && newCost < originalCost - 1e-4) return true;
    else {
      repair.undo();
      destroy.undo();
    }

    ditchs[edgeToDrop.id] = 0;
    if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 0;
  }

  return false;
} 

/**
 * @brief Implementation of the Variable Neighborhood Descent (VNS) Local Search.
 */
bool VNS::optimize(SFPSolution* solution, std::mt19937& rng) {
  if (!dijkstra)
    dijkstra = std::make_shared<DijkstraEngine>(solution->getProblem()->getGraphPtr());
  
  int nEdgesGraph = solution->getProblem()->getNEdges();
  std::vector<uint8_t> ditchs(nEdgesGraph, 0);

  std::vector<SolutionEdge> shuffledEdges = solution->getEdges();
  if (shuffledEdges.empty()) return false;

  int N = (int) shuffledEdges.size();
  if (N == 0) return false;
  
  int k1 = std::max(2, (int)(N * 0.05));
  int k2 = std::max(k1 + 1, (int)(N * 0.10));
  int k3 = std::max(k2 + 1, (int)(N * 0.15));
  int k4 = std::max(k3 + 1, (int)(N * 0.20));
  
  int k_steps[] = {1, k1, k2, k3, k4};

  std::unordered_set<int> affectedPairs;
  affectedPairs.reserve(solution->getNPairs());
  SFPNeighborhood destroy;
  destroy.moves.reserve(solution->getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(solution->getNPairs());
  std::vector<int> pairsToRepair;
  pairsToRepair.reserve(solution->getNPairs());

  for (int k : k_steps) {
    if (k > N) k = N; 

    int attempts = (k == 1) ? N : std::min(10, N);
    
    std::shuffle(shuffledEdges.begin(), shuffledEdges.end(), rng);
    
    for(int att = 0; att < attempts; att++){
      double originalCost = solution->getCurrentCost();
      
      affectedPairs.clear();    
      destroy.moves.clear();
      pairsToRepair.clear();
      repair.moves.clear();
      
      if (k > 1) std::shuffle(shuffledEdges.begin(), shuffledEdges.end(), rng);
      else std::swap(shuffledEdges[0], shuffledEdges[att]);
       
   
      for (int i = 0; i < k; i++) { 
        const auto& edgeToDrop = shuffledEdges[i];
        if (!solution->isEdgeActive(edgeToDrop.id)) continue;
   
        ditchs[edgeToDrop.id] = 1;  
        if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 1;
   
        // LEVEL 1 (The Epicenter): Pairs that pass exactly through the removed edge
        std::vector<int> currentWave; 
        for (int pair_id : solution->getEdgePairs(edgeToDrop))
          if (affectedPairs.insert(pair_id).second) {
            destroy.moves.push_back({solution, MoveType::DSCNCT_PAIR, pair_id, solution->getPairEdges(pair_id)});
            currentWave.push_back(pair_id);
          } 
   
        // LEVELS 2 TO DELTA (The Shock Wave): Cascading Competitors
        for (int d = 2; d <= delta; ++d) {
          std::vector<int> nextWave; 
          for (int p : currentWave) 
            for (int comp : solution->getCompetingPairs(p))
              if (affectedPairs.insert(comp).second) {
                destroy.moves.push_back({solution, MoveType::DSCNCT_PAIR, comp, solution->getPairEdges(comp)});
                nextWave.push_back(comp);
              }
          if (nextWave.empty()) break; 
          currentWave = std::move(nextWave);
        }
      }
      
      destroy.apply();
      bool feasible = true;
      pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
      std::shuffle(pairsToRepair.begin(), pairsToRepair.end(), rng);

      for (int pair : pairsToRepair) {
        auto [source, target] = solution->getPairNodes(pair);
        auto result = dijkstra->getShortPath(source, target, solution->getBitmask(), &ditchs);
        
        if (result.second < 0) {
          feasible = false;
          break;
        }

        repair.addMoveApplying({solution, MoveType::CNCT_PAIR, pair, std::move(result.first)});

        if (solution->getCurrentCost() >= originalCost - 1e-4) {
          feasible = false;
          break;
        }
      }

      double newCost = solution->getCurrentCost();

      for (int i = 0; i < k; i++) {
        ditchs[shuffledEdges[i].id] = 0;
        if (shuffledEdges[i].reverse_id != -1) ditchs[shuffledEdges[i].reverse_id] = 0;
      }

      if (feasible && newCost < originalCost - 1e-4) return true;
      
      repair.undo();
      destroy.undo();
      if (k == 1) std::swap(shuffledEdges[0], shuffledEdges[att]);
    }
  }
  return false;
}
