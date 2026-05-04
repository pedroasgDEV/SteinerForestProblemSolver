#include "Solver.hpp"

#include <chrono>

// Utility: Geodesic Bounding Box Calculator
inline int AMVNS::calculateHopBox(SFPSolution& currentSol, const int source, const int target) const {
    return 20; 
}

/**
 * @brief Diversification Phase (Macro Shockwave)
 */
bool AMVNS::diversification(SFPSolution& currentSol, std::vector<EdgeState>& status, 
                            std::vector<uint8_t>& ditchs, const double W_max) const {
  
  std::vector<const SolutionEdge*> unstable_ptrs;
  for (const auto& edge : currentSol.getEdges())
      if (status[edge.id] == EdgeState::UNSTABLE) {
          edge.I_e = edge.weight / (W_max * std::sqrt(std::max((size_t)1, edge.pairs.size()))); 
          unstable_ptrs.push_back(&edge);
      }

  int N_unstable = unstable_ptrs.size();
  if(N_unstable == 0) return false;

  // Prioritizes Top-Down attack on the worst inefficiencies (bellies)
  std::sort(unstable_ptrs.begin(), unstable_ptrs.end(), 
      [](const SolutionEdge* a, const SolutionEdge* b) { return a->I_e > b->I_e; });

  int k_steps[] = {1, std::max(2, (int)(N_unstable * 0.05)), std::max(3, (int)(N_unstable * 0.10)), 
                   std::max(4, (int)(N_unstable * 0.15)), std::max(5, (int)(N_unstable * 0.20))};
  
  SFPNeighborhood destroy;
  destroy.moves.reserve(currentSol.getNPairs());
  std::unordered_set<int> affectedPairs;
  affectedPairs.reserve(currentSol.getNPairs());
  std::vector<int> pairsToRepair;
  pairsToRepair.reserve(currentSol.getNPairs());
  SFPNeighborhood repair;
  repair.moves.reserve(currentSol.getNPairs());
  
  for (int k : k_steps) {
    if (k > N_unstable) k = N_unstable;

    double originalCost = currentSol.getCurrentCost();
    destroy.moves.clear();
    affectedPairs.clear();
    repair.moves.clear();
    pairsToRepair.clear();

    std::vector<const SolutionEdge*> target_block;
    if (k == 1) target_block.push_back(unstable_ptrs[0]);
    else {
        std::uniform_int_distribution<> dis(0, std::max(0, N_unstable - k) / 2); // Skew towards the first half
        int start_idx = dis(rng);
        for(int i = 0; i < k; ++i) target_block.push_back(unstable_ptrs[start_idx + i]);
    }

    // Phase: Destroy with Dynamic Delta
    for (const auto* edgeToDrop : target_block) {
        if (!currentSol.isEdgeActive(edgeToDrop->id)) continue;

        ditchs[edgeToDrop->id] = 1;
        if (edgeToDrop->reverse_id != -1) ditchs[edgeToDrop->reverse_id] = 1;

        int adaptive_delta = std::max(1, (int) std::ceil(edgeToDrop->I_e * currentSol.getEdgePairs(*edgeToDrop).size()));

        std::vector<int> currentWave;
        for (int pair_id : currentSol.getEdgePairs(*edgeToDrop))
            if (affectedPairs.insert(pair_id).second) {
                currentWave.push_back(pair_id);
                currentSol.snapshotPairCost(pair_id);
                destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, pair_id, currentSol.getPairEdges(pair_id)});
            }
        
        for (int d = 2; d <= adaptive_delta; ++d) {
            std::vector<int> nextWave;
            for (int p : currentWave) 
                for (int comp : currentSol.getCompetingPairs(p)) 
                    if (affectedPairs.insert(comp).second) {
                        currentSol.snapshotPairCost(comp);
                        nextWave.push_back(comp);
                        destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, comp, currentSol.getPairEdges(comp)});
                    }
            if (nextWave.empty()) break; 
            currentWave = std::move(nextWave);
        }
    }

    destroy.apply();
    bool feasible = true;
    pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
    
    // Sort pairs Top-Down (Most expensive first)
    std::sort(pairsToRepair.begin(), pairsToRepair.end(), 
        [&currentSol](int a, int b) { return currentSol.getPairPreviousCost(a) > currentSol.getPairPreviousCost(b); });

    for (int pair : pairsToRepair) {
        auto [source, target] = currentSol.getPairNodes(pair);
        int hopLimit = calculateHopBox(currentSol, source, target);
        
        // Hop-bounded Dijkstra
        auto result = dijkstra->getShortPath(source, target, currentSol.getBitmask(), &ditchs, hopLimit);
        
        if (result.second < 0) { feasible = false; break; }

        repair.addMoveApplying({&currentSol, MoveType::CNCT_PAIR, pair, std::move(result.first)});
        
        if (currentSol.getCurrentCost() >= originalCost - 1e-4) { feasible = false; break; }
    }

    // Clean Ditchs
    for (const auto* e : target_block) {
        ditchs[e->id] = 0;
        if (e->reverse_id != -1) ditchs[e->reverse_id] = 0;
    }

    if (feasible && currentSol.getCurrentCost() < originalCost - 1e-4) return true; 
    else {
        repair.undo();
        destroy.undo();
        
        // Probabilistic Relegation only on k=1 surgery
        if (k == 1) {
            std::uniform_real_distribution<double> dist_prob(0.0, 1.0);
            // Macro failed, demote to micro
            if (dist_prob(rng) < (1.0 - target_block[0]->I_e))
                status[target_block[0]->id] = EdgeState::RESILIENT; 
        }
    }
  }
  return false;
}

/**
 * @brief Intensification Phase (Micro Refinement with Strict Eviction)
 */
void AMVNS::intensification(SFPSolution& currentSol, std::vector<EdgeState>& status, 
                            std::vector<uint8_t>& ditchs, const double I_mean) const {
  bool improved = true;
  std::vector<const SolutionEdge*> resilient_ptrs;
  std::unordered_set<int> affectedPairs;
  SFPNeighborhood destroy;
  SFPNeighborhood repair;
  std::vector<int> pairsToRepair;

  while (improved) {
    improved = false;
    resilient_ptrs.clear();

    for (const auto& edge : currentSol.getEdges())
      if (status[edge.id] == EdgeState::RESILIENT) resilient_ptrs.push_back(&edge);
    
    std::shuffle(resilient_ptrs.begin(), resilient_ptrs.end(), rng);

    for (const auto* edgeToDrop : resilient_ptrs) {
      if (!currentSol.isEdgeActive(edgeToDrop->id)) continue;
      double originalCost = currentSol.getCurrentCost();

      ditchs[edgeToDrop->id] = 1;
      if (edgeToDrop->reverse_id != -1) ditchs[edgeToDrop->reverse_id] = 1;

      affectedPairs.clear(); 
      destroy.moves.clear(); 
      pairsToRepair.clear(); 
      repair.moves.clear();

      int adaptive_delta = std::max(1, (int)std::ceil(edgeToDrop->I_e * currentSol.getEdgePairs(*edgeToDrop).size()));

      std::vector<int> currentWave;
      for (int pair_id : currentSol.getEdgePairs(*edgeToDrop))
        if (affectedPairs.insert(pair_id).second) {
          destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, pair_id, currentSol.getPairEdges(pair_id)});
          currentWave.push_back(pair_id);
          currentSol.snapshotPairCost(pair_id);
        }

      for (int d = 2; d <= adaptive_delta; ++d) {
        std::vector<int> nextWave;
        for (int p : currentWave)
          for (int comp : currentSol.getCompetingPairs(p))
            if (affectedPairs.insert(comp).second) {
              destroy.moves.push_back({&currentSol, MoveType::DSCNCT_PAIR, comp, currentSol.getPairEdges(comp)});
              nextWave.push_back(comp);
              currentSol.snapshotPairCost(comp);
            }
        if (nextWave.empty()) break; 
        currentWave = std::move(nextWave);
      }

      destroy.apply();

      bool feasible = true;
      pairsToRepair.assign(affectedPairs.begin(), affectedPairs.end());
      std::sort(pairsToRepair.begin(), pairsToRepair.end(), 
        [&currentSol](int a, int b) { return currentSol.getPairPreviousCost(a) > currentSol.getPairPreviousCost(b); });
      
      for (int pair : pairsToRepair) {
        auto [source, target] = currentSol.getPairNodes(pair);
        int hopLimit = calculateHopBox(currentSol, source, target);
        auto result = dijkstra->getShortPath(source, target, currentSol.getBitmask(), &ditchs, hopLimit);

        if (result.second < 0) { feasible = false; break; }
        
        repair.addMoveApplying({&currentSol, MoveType::CNCT_PAIR, pair, std::move(result.first)});

        if (currentSol.getCurrentCost() >= originalCost - 1e-4) { feasible = false; break; }
      }

      ditchs[edgeToDrop->id] = 0;
      if (edgeToDrop->reverse_id != -1) ditchs[edgeToDrop->reverse_id] = 0;

      if (feasible && currentSol.getCurrentCost() < originalCost - 1e-4) {
        improved = true;
        break; 
      } 
      else {
        repair.undo();
        destroy.undo();
        
        // Final Memory Sieve (Eviction Rule)
        if (edgeToDrop->I_e < I_mean) status[edgeToDrop->id] = EdgeState::TABU;
        else status[edgeToDrop->id] = EdgeState::UNSTABLE;
      }
    }  
  }  
}

/**
 * @brief Executes the Adaptive Memory Variable Neighborhood Search (AM-VNS).
 */
SFPSolution AMVNS::solve() const {
  
  SFPSolution globalBest = constructive->generate(problem, rng);
  firstCost = globalBest.getCurrentCost();
  
  SFPSolution currentSol = problem->empty_solution();
  currentSol = globalBest;
  
  int nEdgesGraph = problem->getNEdges();   
  std::vector<uint8_t> ditchs(nEdgesGraph, 0);
  std::vector<EdgeState> status(nEdgesGraph, EdgeState::UNSTABLE);

  double W_max = problem->getMaxEdgeWeight();
  if(W_max == 0.0) W_max = 1.0; // Failsafe

  int stagnant_cycles = 0;
  auto start_time = std::chrono::steady_clock::now();
  std::uniform_real_distribution<double> dist_prob(0.0, 1.0);

  while (true) {
    // Time and Stagnation Stop
    auto current_time = std::chrono::steady_clock::now();
    int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
    
    bool exceededTime = (timeLimitSeconds > 0) && (elapsed_seconds >= timeLimitSeconds);
    bool exceededStagnation = (stagnant_cycles >= maxStagnationCycles);

    if (exceededTime || exceededStagnation) break;

    double previous_cycle_cost = currentSol.getCurrentCost();

    // Autonomous Thermometer & Global Rescue
    int count_resilient = 0, count_total = 0;
    for(const auto& edge : currentSol.getEdges()) {
        count_total++;
        if(status[edge.id] == EdgeState::RESILIENT) count_resilient++;
    }
    
    double S_current = (double)count_resilient / std::max(1, count_total);

    for (const auto& edge : currentSol.getEdges())
        if(status[edge.id] == EdgeState::RESILIENT && dist_prob(rng) < S_current)
            status[edge.id] = EdgeState::UNSTABLE;

    // Active Amnesia (Soft-Tabu)
    double decay_factor = 0.05;
    for (const auto& edge : currentSol.getEdges()) 
        if(status[edge.id] == EdgeState::TABU && dist_prob(rng) < (S_current * decay_factor))
            status[edge.id] = EdgeState::UNSTABLE;

    // Dynamic Cutoff (I_mean)
    double sum_I_e = 0.0;
    int count_unstable = 0;
    for (const auto& edge : currentSol.getEdges())
        if (status[edge.id] == EdgeState::UNSTABLE) {
            sum_I_e += edge.weight / (W_max * std::sqrt(std::max((size_t)1, edge.pairs.size())));
            count_unstable++;
        }

    double I_mean = sum_I_e / std::max(1, count_unstable);

    bool macro_improvement = diversification(currentSol, status, ditchs, W_max);
    
    if (macro_improvement) intensification(currentSol, status, ditchs, I_mean);

    // Global Update & Graceful Degradation
    if (currentSol.getCurrentCost() < globalBest.getCurrentCost()) globalBest = currentSol;

    double fractional_gap = (previous_cycle_cost - currentSol.getCurrentCost()) / previous_cycle_cost;
    if (fractional_gap <= 1e-5) stagnant_cycles++;
    else stagnant_cycles = 0; 
  }

  return globalBest;
}
