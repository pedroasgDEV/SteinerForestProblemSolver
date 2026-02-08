#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <string>

#include "../models/SFP.hpp"

/**
 * @class ConstructiveStrategy
 * @brief Interface for algorithms that generate a solution from scratch.
 */
class ConstructiveStrategy {
 public:
  virtual ~ConstructiveStrategy() = default;

  /**
   * @brief Generates a new solution for the given problem.
   * @param problem The static problem definition.
   * @return SFPSolution A new (potentially feasible) solution.
   */
  virtual SFPSolution generate(const SFPProblem& problem) const = 0;

  virtual std::string getName() const = 0;
};

/**
 * @class LocalSearchStrategy
 * @brief Interface for algorithms that refine an existing solution.
 */
class LocalSearchStrategy {
 public:
  virtual ~LocalSearchStrategy() = default;

  /**
   * @brief Tries to improve the given solution in-place.
   * @param solution The solution to be optimized. Modified by reference.
   * @return true If the solution was improved.
   * @return false If no improvement was found (local optimum reached).
   */
  virtual bool optimize(SFPSolution& solution) const = 0;

  virtual std::string getName() const = 0;
};

/**
 * @class SolverStrategy
 * @brief Main Interface for complete solvers (Facade/Runner).
 */
class SolverStrategy {
 public:
  virtual ~SolverStrategy() = default;

  /**
   * @brief Solves the SFP instance completely.
   * @param problem The problem to solve.
   * @return SFPSolution The best solution found.
   */
  virtual SFPSolution solve(const SFPProblem& problem) const = 0;

  virtual std::string getName() const = 0;
};

/**
 * @class GRASPConstructiveHeuristic
 * @brief Implements a Randomized Greedy Constructive Heuristic (RCL based).
 */
class GRASPConstructiveHeuristic : public ConstructiveStrategy {
 private:
  float alpha;  ///< Restricted Candidate List (RCL) parameter [0.0, 1.0].

 public:
  /**
   * @brief Construct a new GRASP Constructive Heuristic.
   * @param alpha parameter controlling greediness.
   * - 0.0: Pure Greedy (Best path always).
   * - 1.0 [default]: Pure Random (Random path selection).
   */
  GRASPConstructiveHeuristic(const float alpha = 1.0f)
      : alpha(alpha) {};

  /**
   * @brief Generates a solution using the candidate list logic.
   * @param problem The SFP instance.
   * @return SFPSolution The constructed solution.
   */
  SFPSolution generate(const SFPProblem& problem) const override;

  /**
   * @brief Returns the algorithm name and configuration.
   */
  std::string getName() const override {
    return "GRASP Constructive (alpha=" + std::to_string(alpha) + ")";
  }
};

/**
 * @class GRASPLocalSearch
 * @brief Implements the "Destroy and Repair" Local Search.
 */
class GRASPLocalSearch : public LocalSearchStrategy {
 public:
  GRASPLocalSearch() = default;

  /**
   * @brief Iteratively removes edges and tries to reconnect terminals with lower costs.
   * @param solution The solution to be refined.
   * @return true if global improvement was achieved.
   */
  bool optimize(SFPSolution& solution) const override;

  std::string getName() const override {
    return "GRASP Local Search";
  }
};

/**
 * @class GRASPMetaheuristic
 * @brief The conductor of the GRASP procedure.
 * * Orchestrates the loop: Construction -> Local Search -> Best Update.
 */
class GRASPMetaheuristic : public SolverStrategy {
 private:
  int maxIterations;
  std::unique_ptr<ConstructiveStrategy> constructive;
  std::unique_ptr<LocalSearchStrategy> localSearch;

 public:
  /**
   * @brief Constructs the GRASP solver.
   * @param maxIter Number of iterations.
   * @param c The constructive strategy (e.g., GRASPConstructiveHeuristic).
   * @param ls The local search strategy (e.g., GRASPLocalSearch).
   */
  GRASPMetaheuristic(int maxIter,
                     std::unique_ptr<ConstructiveStrategy> c,
                     std::unique_ptr<LocalSearchStrategy> ls)
      : maxIterations(maxIter),
        constructive(std::move(c)),
        localSearch(std::move(ls)) {}

  SFPSolution solve(const SFPProblem& problem) const override;

  std::string getName() const override {
    return "GRASP Metaheuristic (" + std::to_string(maxIterations) + " iters)";
  }
};

#endif
