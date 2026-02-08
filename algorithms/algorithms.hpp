#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <queue>
#include <utility>
#include <vector>

#include "../utils/Graph.hpp"


/**
 * @brief GRASP Constructive Heuristic.
 * @param originalGraph SFP initial graph.
 * @param terminals Pairs of terminals that need to be connected.
 * @param alpha RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
 * @return Graph with the SFP solution.
 */
Graph GRASPconstructiveHeuristic(
    const Graph& originalGraph,
    const std::vector<std::pair<int, int>>& terminals, float alpha = 1.0f);

#endif
