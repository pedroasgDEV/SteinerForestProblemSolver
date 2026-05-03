#include <memory>
#include <string>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "tests/Tests.hpp"
#include "models/SFP.hpp"
#include "algorithms/Solver.hpp"
#include "utils/CLI11.hpp"

std::string getFileName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) return path.substr(lastSlash + 1);
    return path;
}

bool hasExtension(const std::string& str, const std::string& suffix) {
    std::string tmp_str = getFileName(str);
    if (tmp_str.length() < suffix.length()) return false;
    return tmp_str.compare(tmp_str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void panic(const std::string& msg) {
    std::cerr << "\n========================================" << std::endl;
    std::cerr << "[FATAL ERROR]: " << msg << std::endl;
    std::cerr << "========================================\n" << std::endl;
    std::exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  CLI::App app{"Steiner Forest Problem Solver"};

  bool flag_test_all = false;
  bool flag_test_graph = false;
  bool flag_test_DSU = false;
  bool flag_test_dijkstra = false;
  bool flag_test_SFP = false;

  app.add_flag("--test", flag_test_all, "Runs all available tests");
  app.add_flag("--test-graph", flag_test_graph,
               "Runs only the Graph struct tests");
  app.add_flag("--test-DSU", flag_test_DSU, "Runs only the DSU struct tests");
  app.add_flag("--test-dijkstra", flag_test_dijkstra,
               "Runs only the Dijkstra algorithm tests");
  app.add_flag("--test-sfp", flag_test_SFP,
               "Runs only the Steiner Forest Problem implementation tests");

  std::string input_file;
  float alpha = 1.0f;
  int maxIter = 1;
  bool flag_irace = false;
  bool flag_grasp = false;
  bool flag_amvns = false;
  bool flag_vns = false;
  int delta1 = 1;
  int delta2 = 1;

  app.add_option("-d,--delta1", delta1, "Delta parameter (Shockwave Level) for the Local Search / Diversification phase")
      ->check(CLI::Range(0, 5));
  app.add_option("-D,--delta2", delta2, "Delta parameter for AMVNS Intensification phase")
      ->check(CLI::Range(0, 5));
  app.add_flag("--IRACE", flag_irace, "Runs for tuning");
  app.add_flag("--GRASP", flag_grasp, "Runs GRASP-SFP metaheuristic");
  app.add_flag("--AMVNS", flag_amvns, "Runs AM-VNS metaheuristic");
  app.add_flag("--VNS", flag_vns, "Runs GRASP-VNS-SFP metaheuristic");
  app.add_option("-f,--file", input_file, "Path to a single .stp file to solve")
      ->check(CLI::ExistingFile);
  app.add_option("-a,--alpha", alpha, "Alpha parameter for constructive heuristic")
      ->check(CLI::Range(0.0, 1.0));
  app.add_option("-i,--iterations", maxIter, "The limit of iterations of the metaheuristic")
      ->check(CLI::PositiveNumber);

  CLI11_PARSE(app, argc, argv);
  
  if (flag_test_all || flag_test_graph || flag_test_dijkstra || flag_test_SFP || flag_test_DSU) {
    if (flag_test_all) {
      graphTests();
      dijkstraTests();
      dsuTests();
      steinerForestTests();
      return 0;
    }
    if (flag_test_graph) graphTests();
    if (flag_test_dijkstra) dijkstraTests();
    if (flag_test_DSU) dsuTests();
    if (flag_test_SFP) steinerForestTests();
    return 0;
  }
  else if(!input_file.empty()){
    if(!hasExtension(input_file, ".stp")){ panic("The file extension must be '.stp'."); }

    std::ifstream file(input_file);
    if (!file.is_open()) { panic("The file cannot be opened"); }
    
    SFPProblem problem;
    try { file >> problem;} 
    catch (const std::exception& e) { panic("Error parsing file\n" + std::string(e.what())); }
    
    double firstSolutionCost = 0.0f, solutionCost = 0.0f, timeMs = 0.0f; 
    if(!flag_grasp && !flag_vns && !flag_amvns){
        auto generate = std::make_unique<GRASPConstructiveHeuristic>(nullptr, alpha);
        static std::random_device rd; static std::mt19937 rng(rd()); 
        auto start = std::chrono::high_resolution_clock::now();
        auto solution = generate->generate(&problem, rng);
        auto end = std::chrono::high_resolution_clock::now();
        
        if(!solution.isFeasible()) panic("No valid solution was found.");
        firstSolutionCost = solution.getCurrentCost();
        solutionCost = firstSolutionCost;
        timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    }
    else {
        std::unique_ptr<SolverStrategy> metaheuristic;
        if (flag_grasp) metaheuristic = std::make_unique<Metaheuristics<GRASPConstructiveHeuristic, GRASPLocalSearch>>(&problem, maxIter, alpha, delta1);
        else if (flag_vns) metaheuristic = std::make_unique<Metaheuristics<GRASPConstructiveHeuristic, VNS>>(&problem, maxIter, alpha, delta1);
        else metaheuristic = std::make_unique<AMVNS>(&problem, maxIter, alpha, delta1, delta2);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto solution = metaheuristic->solve();
        auto end = std::chrono::high_resolution_clock::now();

        if(!solution.isFeasible()) panic("No valid solution was found.");
        firstSolutionCost = metaheuristic->getFirstCost();
        solutionCost = solution.getCurrentCost();
        timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    }
    
    if(flag_irace){ std::cout << solutionCost ; return 0; }

    std::string filename = getFileName(input_file);
    int nNodes = problem.getNNodes();
    int nEdges = problem.getNEdges();
    int nTerminals = problem.getTerminals().size();
    float alphaUsed = alpha;

    std::cout << "\n================ EXECUTION SUMMARY ================" << std::endl;
    std::cout << std::left << std::setw(20) << "Instance File:" << filename << std::endl;
    std::cout << std::left << std::setw(20) << "Nodes:"         << nNodes << std::endl;
    std::cout << std::left << std::setw(20) << "Edges:"         << nEdges << std::endl;
    std::cout << std::left << std::setw(20) << "Terminals:"     << nTerminals << std::endl;
    std::cout << std::left << std::setw(20) << "Alpha Used:"    << std::fixed << std::setprecision(2) << alphaUsed << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(20) << "First Solution Cost:" << std::fixed << std::setprecision(4) << firstSolutionCost << std::endl;
    std::cout << std::left << std::setw(20) << "Solution Cost:" << std::fixed << std::setprecision(4) << solutionCost << std::endl;
    std::cout << std::left << std::setw(20) << "Execution Time:" << std::fixed << std::setprecision(3) << timeMs << " ms" << std::endl;
    std::cout << "===================================================\n" << std::endl;
  
    return 0;
  }
  else {
    std::cout << "No input provided. Use --help to see options." << std::endl;
    return 0;
  }
}
