#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <string>

#include "tests.hpp"
#include "../models/SFP.hpp"

// ==========================================
// Concrete Mock Class for Testing
// ==========================================
class ConcreteSteiner : public SteinerForest {
public:
    using SteinerForest::SteinerForest;

    void setManualCost(float c) { 
        this->totalCost = c; 
    }

    void injectNegativeEdge() {
        this->originalGraph.addEdge(0, 1, -5.0f);
    }

    void breakSolutionConnectivity() {
        if (!terminals.empty()) {
            // Must be adjacent 
            solutionGraph.removeEdge(terminals[0].first, terminals[0].second);
        }
    }

    // Exposes references for deep copy verification
    Graph& getMutableOriginal() { return this->originalGraph; }
};

// ==========================================
// Mock Helper Functions for Testing
// ==========================================

/**
 * @brief Mock Heuristic: Just copies the original graph as the solution.
 */
Graph mockHeuristic(const Graph& original, const std::vector<std::pair<int, int>>& terminals, const float alfa = 1) { return original; }

/**
 * @brief Mock Constraint: Always returns true (Pass).
 */
bool mockConstraintPass(const Graph& g) { return true; }

/**
 * @brief Mock Constraint: Always returns false (Fail).
 */
bool mockConstraintFail(const Graph& g) { return false; }

/**
 * @brief Mock Verifier: Always returns true (Pass).
 */
bool mockVerifierPass(const Graph& sol, const std::vector<std::pair<int, int>>& t) { return true; }

/**
 * @brief Mock Verifier: Always returns false (Fail).
 */
bool mockVerifierFail(const Graph& sol, const std::vector<std::pair<int, int>>& t) { return false; }

// ==========================================
// Test Functions
// ==========================================

void testConstructorsAndAssignment() {
    std::cout << "[Steiner Test] Constructors & Deep Copy... ";
    
    Graph g(5, true);
    g.addEdge(0, 1, 10.0f);
    g.addEdge(2, 3, 20.0f);
    std::vector<std::pair<int, int>> terminals = {{0, 1}, {2, 3}};

    // Test parameter constructor
    ConcreteSteiner sf1(g, terminals);
    assert(sf1.getOriginal().getNNode() == 5);
    assert(sf1.getTerminals().size() == 2);
    assert(sf1.getSolution().getNNode() == 0); 

    // Test Deep copy
    ConcreteSteiner sf2 = sf1;
    sf1.getMutableOriginal().addEdge(0, 4, 999.0f); 
    assert(sf2.getOriginal().isAdjacent(0, 4) == false); 
    assert(sf1.getOriginal().isAdjacent(0, 4) == true);

    // Test default constructor
    ConcreteSteiner sf3;
    sf3 = sf1;
    assert(sf3.getOriginal().isAdjacent(0, 4) == true);

    std::cout << "Passed." << std::endl;
}

void testIOParsing() {
    std::cout << "[Steiner Test] IO Parsing (>> <<)... ";

    std::string inputData = R"(
        SECTION Graph
        Nodes 4
        Edges 3
        E 1 2 10
        E 2 3 20
        E 3 4 30
        END
        
        SECTION Terminals
        Terminals 1
        TP 1 4
        END
    )";

    // Test input parser
    std::stringstream ss(inputData);
    ConcreteSteiner sf;
    ss >> sf;
    assert(sf.getOriginal().getNNode() == 4);
    assert(sf.getOriginal().isAdjacent(0, 1) == true);
    assert(sf.getTerminals().size() == 1);
    assert(sf.getTerminals()[0].first == 0);
    assert(sf.getTerminals()[0].second == 3);

    // Test output parser
    std::cout << sf << std::endl;
    sf.solve(mockHeuristic, { mockConstraintPass }, mockVerifierPass);
    std::cout << sf << std::endl;

    std::cout << "Passed." << std::endl;
}

void testSolveTemplate() {
    std::cout << "[Steiner Test] Template Solve Method... ";

    Graph g(3, true);
    g.addEdge(0, 1, 10.0f);
    g.addEdge(1, 2, 10.0f);
    std::vector<std::pair<int, int>> terminals = {{0, 2}};
    
    SteinerForest sf(g, terminals);

    // --- Test successful Execution ---
    try {
        sf.solve(mockHeuristic, { mockConstraintPass }, isAllTerminalsPairsConnected);
        
        assert(sf.getSolveStats() == true);
        assert(sf.getSolution().isAdjacent(0, 1) == true); // Solution should be a copy of original
    } catch (...) {
        assert(false && "Solve threw exception on valid input");
    }

    // --- Test constraint Failure ---
    try {
        sf.solve(mockHeuristic, { mockConstraintFail }, isAllTerminalsPairsConnected);
        assert(false && "Should have thrown exception on constraint failure");
    } catch (const std::runtime_error& e) {
        // Success (Caught expected error)
    }

    // --- Test solution Verification Failure ---
    
    g.removeEdge(0, 1);
    SteinerForest sf2(g, terminals);

    try {
        sf2.solve(mockHeuristic, { mockConstraintPass }, isAllTerminalsPairsConnected);
        assert(false && "Should have thrown exception on verification failure");
    } catch (const std::runtime_error& e) {
        
    }

    std::cout << "Passed." << std::endl;
}

void testComparisons() {
    std::cout << "[Steiner Test] Comparison Operators... ";

    ConcreteSteiner s1, s2;
    s1.setManualCost(10.0f);
    s2.setManualCost(20.0f);

    assert(s1 < s2);
    assert(s1 <= s2);
    assert(s2 > s1);
    assert(s2 >= s1);
    assert(s1 != s2);
    
    s2.setManualCost(10.0f);
    assert(s1 == s2);

    std::cout << "Passed." << std::endl;
}


void steinerForestTests() {
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "    STARTING SteinerForest TEST SUITE   " << std::endl;
    std::cout << "========================================" << std::endl;
    
    testConstructorsAndAssignment();
    testIOParsing();
    testSolveTemplate();
    testComparisons();
    
    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY     " << std::endl;
    std::cout << "========================================" << std::endl;
}