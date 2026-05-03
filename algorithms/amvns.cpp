#include <algorithm>
#include <utility>

#include "Solver.hpp"

/**
 * @brief Diversification Phase (VNS on BAD)
 * @return True if the topology was improved, False otherwise.
 */
bool AMVNS::diversification(SFPSolution& currentSol, std::vector<EdgeState>& status, 
                            std::vector<uint8_t>& ditchs, const std::vector<SolutionEdge>& badCandidates, 
                            const int delta) const {
  
  // Setup Perturbation Gears (k_steps) for VNS
  int N_bad = badCandidates.size();
  if(N_bad == 0) return false;

  int k1 = std::max(2, (int)(N_bad * 0.05));
  int k2 = std::max(k1 + 1, (int)(N_bad * 0.10));
  int k3 = std::max(k2 + 1, (int)(N_bad * 0.15));
  int k4 = std::max(k3 + 1, (int)(N_bad * 0.20));
  int k_steps[] = {1, k1, k2, k3, k4};
  
  // Shuffle the VNS targets to prevent search bias and cycling
  std::vector<SolutionEdge> shuffledEdges = badCandidates;
  std::shuffle(shuffledEdges.begin(), shuffledEdges.end(), rng);

  SFPNeighborhood destroy;
  destroy.moves.reserve(currentSol.getNPairs());
  std::unordered_set<int> affectedPairs;
  affectedPairs.reserve(currentSol.getNPairs());
  std::vector<int> pairsToRepair;
  pairsToRepair.reserve(currentSol.getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(currentSol.getNPairs());
  std::vector<int> goodEdges;
  goodEdges.reserve(currentSol.getNEdges() / 10);
  
  for (int k : k_steps) {
    if (k > N_bad) k = N_bad;
    
    // k=1 explores ALL bad edges. k>1 explores 10 random blocks.
    int attempts = (k == 1) ? N_bad : std::min(10, N_bad);

    std::shuffle(shuffledEdges.begin(), shuffledEdges.end(), rng);
    
    for (int att = 0; att < attempts; att++) {
      double originalCost = currentSol.getCurrentCost();

      destroy.moves.clear();
      affectedPairs.clear();
      repair.moves.clear();
      pairsToRepair.clear();
      goodEdges.clear();

      // Swap the target edge to the front for k=1 (O(1) testing), shuffle for k>1
      if (k > 1) std::shuffle(shuffledEdges.begin(), shuffledEdges.end(), rng);
      else std::swap(shuffledEdges[0], shuffledEdges[att]);

      // Destroy Phase
      for (int i = 0; i < k; i++) {
        const auto& edgeToDrop = shuffledEdges[i];
        if (!currentSol.isEdgeActive(edgeToDrop.id)) continue;

        ditchs[edgeToDrop.id] = 1;
        if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 1;

        // LEVEL 1 (The Epicenter): Pairs that pass exactly through the removed edge.
        std::vector<int> currentWave;
        for (int pair_id : currentSol.getEdgePairs(edgeToDrop))
          if (affectedPairs.insert(pair_id).second) {
            currentWave.push_back(pair_id);
            destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, pair_id, currentSol.getPairEdges(pair_id)});
          }
        
        // LEVELS 2 TO DELTA (The Shock Wave): Cascading Competitors.
        for (int d = 2; d <= delta; ++d) {
          std::vector<int> nextWave;
          
          // For each pair affected in the previous level, attack its competitors.
          for (int p : currentWave) 
            for (int comp : currentSol.getCompetingPairs(p)) 
              if (affectedPairs.insert(comp).second) {
                nextWave.push_back(comp);
                destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, comp, currentSol.getPairEdges(comp)});
              }
          
          // If the wave hasn't encountered any new competitors, abort early.
          if (nextWave.empty()) break; 
          currentWave = std::move(nextWave);
        }
      }

      destroy.apply();

      bool feasible = true;
      pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
      std::shuffle(pairsToRepair.begin(), pairsToRepair.end(), rng);

      for (int pair : pairsToRepair) {
        auto [source, target] = currentSol.getPairNodes(pair);
        auto result = dijkstra->getShortPath(source, target, currentSol.getBitmask(), &ditchs);
        
        if (result.second < 0) {
          feasible = false;
          break;
        }

        // Track strictly new edges injected by the repair phase
        for (int newEdge : result.first)
          if (!currentSol.isEdgeActive(newEdge)) goodEdges.push_back(newEdge);

        repair.addMoveApplying({&currentSol, MoveType::CNCT_PAIR, pair, std::move(result.first)});
        
        if (currentSol.getCurrentCost() >= originalCost - 1e-4) {
          feasible = false;
          break;
        }
      }

      double newCost = currentSol.getCurrentCost();

      for (int i = 0; i < k; i++) {
        ditchs[shuffledEdges[i].id] = 0;
        if (shuffledEdges[i].reverse_id != -1) ditchs[shuffledEdges[i].reverse_id] = 0;
      }

      // The topology evolved. Mark new shortcuts as GOOD
      if (feasible && newCost < originalCost - 1e-4) {
        for (int newEdge : goodEdges) status[newEdge] = EdgeState::GOOD;
        if (k == 1) std::swap(shuffledEdges[0], shuffledEdges[att]); // Restore swap
        return true; 
      } 
      else {
        // Failure. Undo the changes
        repair.undo();
        destroy.undo();
        
        // Blame Assignment
        if (k == 1) status[shuffledEdges[0].id] = EdgeState::UGLY;
      }

      if (k == 1) std::swap(shuffledEdges[0], shuffledEdges[att]);
    }
  }
  return false;
}

/**
 * @brief Intensification Phase (Local Search on UGLY)
 */
void AMVNS::intensification(SFPSolution& currentSol, std::vector<EdgeState>& status, 
                            std::vector<uint8_t>& ditchs, const int delta) const {
  bool ugliesImproved = true;
  std::vector<SolutionEdge> uglyCandidates;
  uglyCandidates.reserve(currentSol.getNEdges());
  std::unordered_set<int> affectedPairs;
  affectedPairs.reserve(currentSol.getNPairs());
  SFPNeighborhood destroy;
  destroy.moves.reserve(currentSol.getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(currentSol.getNPairs());
  std::vector<int> pairsToRepair;
  pairsToRepair.reserve(currentSol.getNPairs());
  std::vector<int> goodEdges;
  goodEdges.reserve(currentSol.getNEdges() / 10);

  while (ugliesImproved) {
    ugliesImproved = false;
    uglyCandidates.clear();

    for (const auto& edge : currentSol.getEdges())
      if (status[edge.id] == EdgeState::UGLY) uglyCandidates.push_back(edge);
    
    // Shuffle the UGLY candidates to break cycling
    std::shuffle(uglyCandidates.begin(), uglyCandidates.end(), rng);

    for (const auto& edgeToDrop : uglyCandidates) {
      if (!currentSol.isEdgeActive(edgeToDrop.id)) continue;
      double originalCost = currentSol.getCurrentCost();

      ditchs[edgeToDrop.id] = 1;
      if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 1;

      affectedPairs.clear();
      destroy.moves.clear();
      pairsToRepair.clear();
      repair.moves.clear();
      goodEdges.clear();

      // LEVEL 1 (The Epicenter): Pairs that pass exactly through the removed edge.
      std::vector<int> currentWave;
      for (int pair_id : currentSol.getEdgePairs(edgeToDrop))
        if (affectedPairs.insert(pair_id).second) {
          destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, pair_id, currentSol.getPairEdges(pair_id)});
          currentWave.push_back(pair_id);
        }

      // LEVELS 2 TO DELTA (The Shock Wave): Cascading Competitors.
      for (int d = 2; d <= delta; ++d) {
        std::vector<int> nextWave;
        for (int p : currentWave)
          for (int comp : currentSol.getCompetingPairs(p))
            if (affectedPairs.insert(comp).second) {
              destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, comp, currentSol.getPairEdges(comp)});
              nextWave.push_back(comp);
            }
          
        if (nextWave.empty()) break; 
        currentWave = std::move(nextWave);
      }

      destroy.apply();
      bool feasible = true;
      pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
      std::shuffle(pairsToRepair.begin(), pairsToRepair.end(), rng);
      
      for (int pair : pairsToRepair) {
        auto [source, target] = currentSol.getPairNodes(pair);
        auto result = dijkstra->getShortPath(source, target, currentSol.getBitmask(), &ditchs);

        if (result.second < 0) {
          feasible = false;
          break;
        }

        for(auto newEdge : result.first) 
          if (!currentSol.isEdgeActive(newEdge)) goodEdges.push_back(newEdge);


        repair.addMoveApplying({&currentSol, MoveType::CNCT_PAIR, pair, std::move(result.first)});
        
        if (currentSol.getCurrentCost() >= originalCost - 1e-4) {
          feasible = false;
          break;
        }
      }

      ditchs[edgeToDrop.id] = 0;
      if (edgeToDrop.reverse_id != -1) ditchs[edgeToDrop.reverse_id] = 0;

      // The edge is no longer ugly, the topology has changed
      if (feasible && currentSol.getCurrentCost() < originalCost - 1e-4) {
        ugliesImproved = true;

        for (int newEdge : goodEdges)
          status[newEdge] = EdgeState::GOOD;
        
        break; 
      } 
      // Edge remains UGLY. Failure, undo the changes
      else {
        repair.undo();
        destroy.undo();
      }
    }  
  }  
}

/**
 * @brief Executes the Adaptive Memory Variable Neighborhood Search (AM-VNS).
 * * Strategy: Iterated Local Search with State Partitioning (BAD, GOOD, UGLY).
 * 1. Generates an initial solution via GRASP.
 * 2. Diversification: Applies VNS on the 'BAD' edge set to break out of local optima.
 * 3. State Transition: Failed single-edge removals are tagged as 'UGLY'.
 * 4. Intensification: If VNS finds an improvement, it triggers a rigorous
 *    First-Improvement Local Search strictly on the 'UGLY' set.
 * 5. Memory Update: 'GOOD' edges are promoted to 'BAD' when the 'BAD' set is depleted.
 */
SFPSolution AMVNS::solve() const {
  
  SFPSolution globalBest = problem->empty_solution();
  int nEdgesGraph = problem->getNEdges();   
  std::vector<uint8_t> ditchs(nEdgesGraph, 0);

  for (int iter = 0; iter < maxIterations; ++iter) {

    SFPSolution currentSol = constructive->generate(problem, rng);
    if(iter == 0) firstCost = currentSol.getCurrentCost();

    std::vector<EdgeState> status(nEdgesGraph, EdgeState::BAD);
    for (const auto& edge : currentSol.getEdges()) status[edge.id] = EdgeState::BAD;
    
    bool runSearch = true;
    std::vector<SolutionEdge> badCandidates; 
    badCandidates.reserve(currentSol.getNEdges());

    while (runSearch) {
      badCandidates.clear();

      for (const auto& edge : currentSol.getEdges())
        if (status[edge.id] == EdgeState::BAD) badCandidates.push_back(edge);

      // Memory Promotion (GOOD -> BAD)
      if (badCandidates.empty()) {
        bool hasGood = false;
        for (const auto& edge : currentSol.getEdges())
          if (status[edge.id] == EdgeState::GOOD) {
            status[edge.id] = EdgeState::BAD;
            badCandidates.push_back(edge);
            hasGood = true;
          }
        
        // Deep Local Optimum Reached: If there is nothing GOOD left to test, end the GBU for this iteration.
        if (!hasGood) {
          runSearch = false;
          break;
        }
        // Restart the while(runSearch) now that BAD has been replenished with the former heroes
        continue; 
      }

      bool improvedInBad = diversification(currentSol, status, ditchs, badCandidates, delta1);
      
      if (improvedInBad) intensification(currentSol, status, ditchs, delta2);

      if (iter == 0 || currentSol.getCurrentCost() < globalBest.getCurrentCost()) globalBest = currentSol;
    }
  }

  return globalBest;
}
