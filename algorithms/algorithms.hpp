#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <vector>
#include "../utils/graph.hpp"

/**
 * @brief Finds the shortest path between two vertices using Dijkstra's Algorithm.
 * @param start_id The ID of the starting vertex (1-based).
 * @param end_id The ID of the destination vertex (1-based).
 * @return std::pair<std::vector<int>, float> A sequence of node IDs representing the shortest path and the cost. 
 * Returns an empty vector if no path exists.
 */
std::pair<std::vector<int>, float> dijkstra(const Graph& graph, int start_id, int end_id);

/**
 * @brief Validates if the solution graph connects all terminal pairs.
 * The solution F must ensure that all vertices of each Ti belong to the same connected component.
 */
bool validateSolution(const Graph& solution, const std::vector<std::pair<int, int>>& terminals);

/**
 * @brief GRASP Constructive Heuristic.
 * @param originalGraph SFP initial graph.
 * @param terminals Pairs of terminals that need to be connected.
 * @param alpha RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
 * @return Graph with the SFP solution.
 */
Graph GRASPconstructiveHeuristic(const Graph& originalGraph, const std::vector<std::pair<int, int>>& terminals, float alpha = 1.0f);

#endif