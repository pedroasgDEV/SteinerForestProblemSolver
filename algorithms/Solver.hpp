#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <cstdint>
#include <limits>
#include <vector>
#include <algorithm>
#include <map>
#include <random>
#include <unordered_map>
#include <type_traits>

#include "../models/SFP.hpp"
#include "../utils/DSU.hpp"
#include "../utils/Dijkstra.hpp"

/**
 * @class ConstructiveStrategy
 * @brief Interface for algorithms that generate a solution from scratch.
 */
class ConstructiveStrategy {
 public:
  virtual ~ConstructiveStrategy() = default;
  virtual SFPSolution generate(const SFPProblem* problem, std::mt19937& rng) = 0;
  virtual std::string getName() const = 0;
};

/**
 * @class LocalSearchStrategy
 * @brief Interface for algorithms that refine an existing solution.
 */
class LocalSearchStrategy {
 public:
  virtual ~LocalSearchStrategy() = default;
  virtual bool optimize(SFPSolution* solution, std::mt19937& rng) = 0;
  virtual std::string getName() const = 0;
};

/**
 * @class SolverStrategy
 * @brief Main Interface for complete solvers (Orchestrators).
 */
class SolverStrategy {
 public:
  virtual ~SolverStrategy() = default;
  virtual SFPSolution solve() const = 0;
  virtual double getFirstCost() const = 0;
  virtual std::string getName() const = 0;
};

/**
 * @class GRASPConstructiveHeuristic
 */
class GRASPConstructiveHeuristic : public ConstructiveStrategy {
 private:
  float alpha;
  mutable std::shared_ptr<DijkstraEngine> dijkstra;

 public:
  GRASPConstructiveHeuristic(std::shared_ptr<DijkstraEngine> externalDijkstra = nullptr, const float alpha = 1.0f) 
      : alpha(alpha), dijkstra(externalDijkstra) {}

  SFPSolution generate(const SFPProblem* problem, std::mt19937& rng) override;
  std::string getName() const override { return "GRASP" + std::to_string(alpha); }
};

/**
 * @class GRASPLocalSearch
 */
class GRASPLocalSearch : public LocalSearchStrategy {
 private:
  mutable std::shared_ptr<DijkstraEngine> dijkstra;
  int delta;

 public:
  GRASPLocalSearch(std::shared_ptr<DijkstraEngine> externalDijkstra = nullptr, int delta = 1) 
      : dijkstra(externalDijkstra), delta(delta) {}

  bool optimize(SFPSolution* solution, std::mt19937& rng) override;
  std::string getName() const override { return "GRASP_LS"; }
};

/**
 * @class Variable Neighborhood Search
 */
class VNS : public LocalSearchStrategy {
 private:
  mutable std::shared_ptr<DijkstraEngine> dijkstra;
  int delta;

 public:
  VNS(std::shared_ptr<DijkstraEngine> externalDijkstra = nullptr, int delta = 1) 
      : dijkstra(externalDijkstra), delta(delta) {}

  bool optimize(SFPSolution* solution, std::mt19937& rng) override;
  std::string getName() const override { return "VNS"; }
};

/**
 * @class Metaheuristics
 * @brief Template solver strategy capable of dynamically combining any Constructive and Local Search.
 */
template <typename Constructive , typename LocalSearch>
class Metaheuristics : public SolverStrategy {
    static_assert(std::is_base_of_v<ConstructiveStrategy, Constructive>, 
                "METAHEURISTIC_ERROR: Constructive type must inherit from ConstructiveStrategy.");
    static_assert(std::is_base_of_v<LocalSearchStrategy, LocalSearch>, 
                "METAHEURISTIC_ERROR: LocalSearch type must inherit from LocalSearchStrategy.");

 private:
  const SFPProblem* problem;
  mutable double firstCost;
  int maxIterations;
  std::unique_ptr<Constructive> constructive;
  std::unique_ptr<LocalSearch> localSearch;
  mutable std::mt19937 rng;

 public:
    Metaheuristics(const SFPProblem* problem, const int maxIter = 1, const float alpha = 1.0f, int delta = 1) 
        : problem(problem), firstCost(-1.0f), maxIterations(maxIter){
       std::random_device rd;
       rng.seed(rd());
       auto dijkstra = std::make_shared<DijkstraEngine>(problem->getGraphPtr()); 
       constructive = std::make_unique<Constructive>(dijkstra, alpha); 
       localSearch = std::make_unique<LocalSearch>(dijkstra, delta); 
    }

    SFPSolution solve() const override {
        SFPSolution best = constructive->generate(problem, rng);
        firstCost = best.getCurrentCost();
        while (localSearch->optimize(&best, rng));
        for(int it = 1; it < maxIterations; it++){
            SFPSolution temp = constructive->generate(problem, rng);
            while (localSearch->optimize(&temp, rng));
            if(best.getCurrentCost() > temp.getCurrentCost()) best = temp;
        }

        return best;
    }
    double getFirstCost() const override { return firstCost; }
    std::string getName() const override { return constructive->getName() + "-" + localSearch->getName() + "-SFP"; }
};


/*
 * @class AM-VNS
 * @brief A Hybrid solver based on Adaptive Memory Variable Neighborhood Search
 */
class AMVNS : public SolverStrategy {
 private:
  
  enum class EdgeState { GOOD, BAD, UGLY };
  
  const SFPProblem* problem;
  mutable double firstCost;
  int maxIterations;
  float alpha;
  int delta1, delta2;
  std::unique_ptr<GRASPConstructiveHeuristic> constructive;
  mutable std::shared_ptr<DijkstraEngine> dijkstra;
  mutable std::mt19937 rng;

  bool diversification(SFPSolution& currentSol, std::vector<EdgeState>& status, std::vector<uint8_t>& ditchs, const std::vector<SolutionEdge>& badCandidates, const int delta = 1) const;
  void intensification(SFPSolution& currentSol, std::vector<EdgeState>& status, std::vector<uint8_t>& ditchs, const int delta = 1) const;

 public:
  AMVNS(const SFPProblem* problem, const int maxIter = 1, const float alpha = 1.0f, const int delta1 = 1, const int delta2 = 1) 
      : problem(problem), firstCost(-1.0f), maxIterations(maxIter), alpha(alpha), delta1(delta1), delta2(delta2){
     std::random_device rd;
     rng.seed(rd());
     dijkstra = std::make_shared<DijkstraEngine>(problem->getGraphPtr()); 
     constructive = std::make_unique<GRASPConstructiveHeuristic>(dijkstra, alpha); 
  }

  SFPSolution solve() const override;
  double getFirstCost() const override { return firstCost; }
  std::string getName() const override { return "AM-VNS"; }
};

#endif
