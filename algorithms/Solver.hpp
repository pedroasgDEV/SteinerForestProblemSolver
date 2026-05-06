#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <random>
#include <memory>
#include <string>

#include "../models/SFP.hpp"
#include "../utils/BidDijkstra.hpp"

/**
 * @class ConstructiveStrategy
 * @brief Interface for algorithms that generate a solution from scratch.
 */
class ConstructiveStrategy {
 public:
  virtual ~ConstructiveStrategy() = default;
  virtual SFPSolution generate(const SFPProblem* problem) = 0;
  virtual std::string getName() const = 0;
};

/**
 * @class LocalSearchStrategy
 * @brief Interface for algorithms that refine an existing solution.
 */
class LocalSearchStrategy {
 public:
  virtual ~LocalSearchStrategy() = default;
  virtual bool optimize(SFPSolution* solution) = 0;
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
  mutable std::shared_ptr<BidirectionalDijkstraEngine> dijkstra;
  std::mt19937& rng;

 public:
  GRASPConstructiveHeuristic(std::mt19937& rng, std::shared_ptr<BidirectionalDijkstraEngine> externalDijkstra = nullptr, const float alpha = 1.0f) 
      : alpha(alpha), dijkstra(externalDijkstra), rng(rng) {}

  SFPSolution generate(const SFPProblem* problem) override;
  std::string getName() const override { return "GRASP" + std::to_string(alpha); }
};

/**
 * @class GRASPLocalSearch
 */
class GRASPLocalSearch : public LocalSearchStrategy {
 private:
  mutable std::shared_ptr<BidirectionalDijkstraEngine> dijkstra;

 public:
  GRASPLocalSearch(std::shared_ptr<BidirectionalDijkstraEngine> externalDijkstra = nullptr) 
      : dijkstra(externalDijkstra) {}

  bool optimize(SFPSolution* solution) override;
  std::string getName() const override { return "GRASP_LS"; }
};

/**
 * @class KeyPathLocalSearch
 */
class HubBreakingLocalSearch : public LocalSearchStrategy {
 private:
  mutable std::shared_ptr<BidirectionalDijkstraEngine> dijkstra;

 public:
  HubBreakingLocalSearch(std::shared_ptr<BidirectionalDijkstraEngine> externalDijkstra = nullptr) 
      : dijkstra(externalDijkstra) {}

  bool optimize(SFPSolution* solution) override;
  std::string getName() const override { return "GRASP_LS"; }
};

/**
 * @class Metaheuristics
 * @brief Template solver strategy capable of dynamically combining any Constructive and Local Search.
 */
template <typename LocalSearch>
class Metaheuristics : public SolverStrategy {
    static_assert(std::is_base_of_v<LocalSearchStrategy, LocalSearch>, 
                "METAHEURISTIC_ERROR: LocalSearch type must inherit from LocalSearchStrategy.");

 private:
  const SFPProblem* problem;
  mutable double firstCost;
  int maxIterations;
  mutable std::mt19937 rng;
  std::unique_ptr<GRASPConstructiveHeuristic> constructive;
  std::unique_ptr<LocalSearch> localSearch;

 public:
    Metaheuristics(SFPProblem* problem, const int maxIter = 1, const float alpha = 1.0f) 
        : problem(problem), firstCost(-1.0f), maxIterations(maxIter) {
       std::random_device rd;
       rng.seed(rd());
       auto dijkstra = std::make_shared<BidirectionalDijkstraEngine>(problem->getGraphPtr()); 
       
       constructive = std::make_unique<GRASPConstructiveHeuristic>(rng, dijkstra, alpha); 
       localSearch = std::make_unique<LocalSearch>(dijkstra); 
    }

    SFPSolution solve() const override {
        SFPSolution best = constructive->generate(problem);
        firstCost = best.getCurrentCost();
        
        while (localSearch->optimize(&best));
        
        for(int it = 1; it < maxIterations; it++){
            SFPSolution temp = constructive->generate(problem);
            while (localSearch->optimize(&temp));
            if(best.getCurrentCost() > temp.getCurrentCost()) best = temp;
        }

        return best;
    }
    double getFirstCost() const override { return firstCost; }
    std::string getName() const override { return constructive->getName() + "-" + localSearch->getName() + "-SFP"; }
};

#endif
