#ifndef STEINER_FOREST_HPP
#define STEINER_FOREST_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <chrono>

#include "../utils/graph.hpp"


/**
 * @brief Function pointer type for checking graph constraints.
 * @param graph The graph to be checked.
 * @return true if the graph satisfies the constraint, false otherwise.
 */
using ConstraintFunc = bool (*)(const Graph&);

/**
 * @brief Function pointer type for a solving strategy.
 * @param originalGraph The input graph of the problem instance.
 * @param terminals The vector of terminal pairs to connect.
 * @return Graph A new graph representing the solution.
 */
using SolveMethodFunc = Graph (*)(const Graph&, const std::vector<std::pair<int, int>>&, const float);

/**
 * @brief Function pointer type for validating a solution.
 * @param terminals The vector of terminal pairs that must be connected.
 * @return true if the solution is valid, false otherwise.
 */
using ValidatorFunc = bool (*)(const Graph&, const std::vector<std::pair<int, int>>&);


/**
 * @class SteinerForest
 * @brief Represents an instance and solution for the Steiner Forest Problem.
 * * This class manages the input graph, the set of terminal pairs that need 
 * connectivity, and the resulting solution subgraph.
 */
class SteinerForest {
protected:
    Graph graph;                        
    std::vector<std::pair<int, int>> terminals;
    float totalCost;                            
    bool solved;

public:

    /**
     * @brief Default Constructor.
     * Initializes empty graphs and 0 cost.
     */
    SteinerForest() : originalGraph(0), solutionGraph(0), terminals(0), totalCost(0.0f), solved(false) {};

    /**
     * @brief Parametric Constructor.
     * Initializes the problem with a given graph and terminal pairs.
     * @param graph The original graph instance.
     * @param terminals The vector of terminal pairs.
     */
    SteinerForest(const Graph& graph, const std::vector<std::pair<int, int>>& terminals)
        : originalGraph(graph), solutionGraph(0), terminals(terminals), totalCost(0.0f), solved(false) {};

    /**
     * @brief Deep Copy Constructor.
     * @param other The object to copy from.
     */
    SteinerForest(const SteinerForest& other) = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~SteinerForest () = default;

    /**
     * @brief Get the original graph object.
     * @return A Graph object.
     */
    Graph getOriginal() const { return originalGraph; }

    /**
     * @brief Get the solution graph object.
     * @return A Graph object with all terminals pairs connected.
     */
    Graph getSolution() const { return solutionGraph; }

    /**
     * @brief Get the terminal nodes pair vector.
     * @return A nodes pair vector.
     */
    std::vector<std::pair<int, int>> getTerminals() const { return terminals; }

    /**
     * @brief Get the total cost of the solution.
     * @return float dum of edge weights.
     */
    float getCost() const { return totalCost; }

    /**
     * @brief Get if the problem has a solution.
     * @return true if is solved false if not.
     */
    bool getSolveStats() const { return solved; }

    /**
     * @brief Solves the Steiner Forest Problem using a specific strategy and measures execution time as a orchestrator.
     * @param alg The strategy to find a solution.
     * @param cstrs A vector of constraint functions to validate the input graph.
     * @param verify A function to validate the generated solution.
     * @param alfa RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
     * @return double The execution time of the algorithm in ms.
     * @throws std::runtime_error If input constraints are violated or the solution is invalid.
     */
    double solve(const SolveMethodFunc alg, const std::vector<ConstraintFunc>& cstrs, const ValidatorFunc verify, const float alfa = 1.0f) {
        
        // Validate Input Constraints
        for(auto cstr : cstrs) {
            if(!cstr(this->originalGraph)) 
                throw std::runtime_error("The original graph does not meet all the constraints");
        }

        auto start = std::chrono::high_resolution_clock::now();

        // Execute Algorithm
        Graph mybSolution = alg(originalGraph, terminals, alfa);

        auto end = std::chrono::high_resolution_clock::now();
        
        // Validate Solution
        if (!verify(mybSolution, terminals)) {
            throw std::runtime_error("The generated solution is invalid according to the verifier.");
        }

        solutionGraph = mybSolution;
        solved = true; totalCost = solutionGraph.getTotalWeight();
        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count();
    }

    /**
     * @brief Deep Copy Assignment Operator.
     * @param other The object to assign from.
     * @return Reference to this object.
     */
    SteinerForest& operator=(const SteinerForest& other) {
        if (this == &other) {
            return *this;
        }

        this->originalGraph = other.originalGraph;
        this->solutionGraph = other.solutionGraph;
        this->terminals = other.terminals;
        this->totalCost = other.totalCost;
        this->solved = other.solved;

        return *this;
    }

    /**
     * @brief Less-than operator based on total cost.
     * Used for sorting solutions (e.g., in a population-based metaheuristic).
     * @param other Another solution to compare.
     * @return true if this cost is lower than other's cost.
     */
    bool operator<(const SteinerForest& other) const { return this->totalCost < other.totalCost; }

    /**
     * @brief Greater-than operator based on total cost.
     * @param other Another solution to compare.
     * @return true if this cost is higher than other's cost.
     */
    bool operator>(const SteinerForest& other) const { return this->totalCost > other.totalCost; }

    /**
     * @brief Less-than-or-equal operator based on total cost.
     * @param other Another solution to compare.
     * @return true if this cost is lower or equal to other's cost.
     */
    bool operator<=(const SteinerForest& other) const { return this->totalCost <= other.totalCost; }

    /**
     * @brief Greater-than-or-equal operator based on total cost.
     * @param other Another solution to compare.
     * @return true if this cost is higher or equal to other's cost.
     */
    bool operator>=(const SteinerForest& other) const { return this->totalCost >= other.totalCost; }

    /**
     * @brief Equality operator based on total cost.
     * @note Compares floating point values directly. 
     * @param other Another solution to compare.
     * @return true if costs are exactly equal.
     */
    bool operator==(const SteinerForest& other) const { return this->totalCost == other.totalCost; }

    /**
     * @brief Inequality operator based on total cost.
     * @param other Another solution to compare.
     * @return true if costs are different.
     */
    bool operator!=(const SteinerForest& other) const { return !(*this == other); }

    /**
     * @brief Overload of the input operator to parse the specific file format.
     * * Format expected:
     * SECTION Graph
     * Nodes <Number of Nodes: Int>
     * Edges <Number of Edges: Int>
     * E <u from Nodes: int> <v from Nodes: int> <Edge Weight: int>
     * ...
     * END
     *
     * SECTION Terminals
     * Terminals <Number of terminals: Int>
     * TP <u from Nodes: int> <v from Nodes: int>
     * ...
     * END
     * @param in Input stream.
     * @param sf The SteinerForest object to populate.
     * @return std::istream& The input stream.
     */
    friend std::istream& operator>>(std::istream& in, SteinerForest& sf) {
        std::string token;
        bool readingGraph = false;
        bool readingTerminals = false;

        while (in >> token) {
            if (token == "SECTION") {
                std::string sectionName;
                in >> sectionName;
                if (sectionName.find("Graph") != std::string::npos) {
                    readingGraph = true;
                    readingTerminals = false;
                } else if (sectionName.find("Terminals") != std::string::npos) {
                    readingGraph = false;
                    readingTerminals = true;
                }
            }
            else if (token == "END") {
                readingGraph = false;
                readingTerminals = false;
            }
            else if (readingGraph) {
                if (token == "Nodes") {
                    int n;
                    in >> n;
                    sf.originalGraph = Graph(n, true);
                }
                else if (token == "Edges") {
                    int m;
                    in >> m;
                }
                else if (token == "E") {
                    int u, v;
                    float w;
                    in >> u >> v >> w;
                    sf.originalGraph.addEdge(u - 1, v - 1, w);
                }
            }
            else if (readingTerminals) {
                if (token == "Terminals") {
                    int k;
                    in >> k;
                }
                else if (token == "TP") {
                    int u, v;
                    in >> u >> v;
                    sf.terminals.push_back({u - 1, v - 1});
                }
            }
        }

        if (in.eof() && sf.originalGraph.getNNode() > 0) in.clear();

        return in;
    }

    /**
     * @brief Overloads the << operator to print the solution.
     * @param out The output stream.
     * @param sf The SteinerForest object.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& out, const SteinerForest& sf) {
        out << std::endl;
        if (!sf.solved) {
            out << "[Steiner Forest] Solution not computed yet." << std::endl;
            return out;
        }
        out << "====================================" << std::endl;
        out << "      STEINER FOREST SOLUTION       " << std::endl;
        out << "====================================" << std::endl;
        out << "Total Cost: " << sf.totalCost << std::endl;
        out << "Terminals Pairs to Connect: " << sf.terminals.size() << std::endl;
        out << "Solution Subgraph Structure: " << std::endl;
        out << sf.solutionGraph;
        out << "====================================" << std::endl;
        return out;
    }
};

//------------------- Solution Validation Functions -------------------

/**
 * @brief Validates if the solution graph connects all terminal pairs.
 * @param solution Graph objs
 * @param terminals Vector with pair of terminals nodes id
 * @return true if all terminals are connected, false if not.
 */
bool isAllTerminalsPairsConnected(const Graph& solution, const std::vector<std::pair<int, int>>& terminals);

#endif
