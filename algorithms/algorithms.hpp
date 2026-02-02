#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <vector>
#include "../utils/graph.hpp"

/**
 * @brief Structure to hold the result of the Dijkstra algorithm.
 */
struct DijkstraResult {
    /** * @brief The reconstructed shortest path.
     * Each element is a pair: <Node ID, Incoming Edge Index>.
     * For the starting node, this value is -1.
     */
    std::vector<std::pair<int, int>> path;
	
    /// The total accumulated weight of the shortest path.
    float cost;
};

/**
 * @brief Finds the shortest path between two vertices using Dijkstra's Algorithm on a CSR Graph.
 * @param graph The CSR Graph object.
 * @param start_id The ID of the starting vertex (0-based).
 * @param end_id The ID of the destination vertex (0-based).
 * @return DijkstraResult A struct containing the path sequence (nodes and edge indices) and the total cost.
 * If no path exists, returns an empty path and infinity cost.
 */
DijkstraResult dijkstra(const Graph& graph, const int start_id, const int end_id);

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
