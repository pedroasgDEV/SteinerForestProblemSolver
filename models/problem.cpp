#include <random>

#include "../utils/Dijkstra.hpp"
#include "SFP.hpp"

SFPProblem::SFPProblem(std::shared_ptr<Graph> g,
                       const std::vector<std::pair<int, int>>& terminals)
    : graph(g), terminals(terminals), instanceName("Manual") {
  if (!g) throw std::runtime_error("Graph pointer cannot be null");
  if (hasNegativeWeights(*g))
    throw std::runtime_error("Graph has negative weights.");
  if (!isGraphConnected(*g))
    throw std::runtime_error("Graph is not connected.");
}

SFPSolution SFPProblem::empty_solution() const { return SFPSolution(*this); }

SFPSolution SFPProblem::random_solution() const {
  SFPSolution sol(*this);
  DSU dsu(getNNodes());
  DijkstraEngine dijkstra(getNNodes());

  // Shuffle the processing order of the terminals.
  std::vector<std::pair<int, int>> shuffledTerminals = terminals;
  static std::random_device rd;
  static std::mt19937 rng(rd());
  std::shuffle(shuffledTerminals.begin(), shuffledTerminals.end(), rng);

  // Construct the solution
  for (const auto& pair : shuffledTerminals) {
    int source = pair.first;
    int target = pair.second;

    if (!dsu.isConnected(source, target)) {
      auto result = dijkstra.getShortPath(*graph, source, target);
      const std::vector<int>& pathEdges = result.first;

      for (int edgeIdx : pathEdges)
        if (!sol.isEdgeActive(edgeIdx)) {
          sol.internalAdd(edgeIdx);

          sol.currentCost += graph->edges[edgeIdx].weight;

          // Update DSU
          const auto& edge = graph->edges[edgeIdx];
          dsu.unite(edge.source, edge.target);
        }
    }
  }

  return sol;
}

std::istream& operator>>(std::istream& in, SFPProblem& sf) {
  std::string token;
  bool readingGraph = false;
  bool readingTerminals = false;
  std::vector<std::tuple<int, int, float>> edgeList;
  int nNodes = 0;

  while (in >> token) {
    if (token == "SECTION") {
      std::string sectionName;
      in >> sectionName;

      if (sectionName.find("Graph") != std::string::npos) {
        readingGraph = true;
        readingTerminals = false;
      }

      else if (sectionName.find("Terminals") != std::string::npos) {
        readingGraph = false;
        readingTerminals = true;
      }
    }

    else if (token == "END") {
      readingGraph = false;
      readingTerminals = false;
    }

    else if (readingGraph) {
      if (token == "Nodes")
        in >> nNodes;

      else if (token == "Edges") {
        int nEdges;
        in >> nEdges;
        edgeList.reserve(nEdges);
      }

      else if (token == "E") {
        int source, target;
        float weight;
        in >> source >> target >> weight;
        // Assume input 1-based, converte para 0-based
        edgeList.push_back({source - 1, target - 1, weight});
      }
    }

    else if (readingTerminals) {
      if (token == "Terminals") {
        int nTerminals;
        in >> nTerminals;
        sf.terminals.reserve(nTerminals);
      } else if (token == "TP") {
        int source, target;
        in >> source >> target;
        sf.terminals.push_back({source - 1, target - 1});
      }
    }
  }

  if (nNodes > 0 && !edgeList.empty()) {
    try {
      sf.graph = std::make_shared<Graph>(edgeList, nNodes);

      if (hasNegativeWeights(*(sf.graph))) {
        std::cerr << "Error: Graph has negative weights.\n";
        in.setstate(std::ios::failbit);
      }

      if (!isGraphConnected(*(sf.graph))) {
        std::cerr << "Error: Graph is disconnected.\n";
        in.setstate(std::ios::failbit);
      }

    }

    catch (const std::exception& e) {
      std::cerr << "Error constructing graph: " << e.what() << std::endl;
      in.setstate(std::ios::failbit);
    }
  }

  else
    in.setstate(std::ios::failbit);

  return in;
}

std::ostream& operator<<(std::ostream& out, const SFPProblem& sf) {
  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  out << std::endl << "SFP Instance: " << sf.instanceName << std::endl;
  out << "Terminals Pairs: " << sf.terminals.size() << std::endl;

  // Delegate to Graph's printer
  if (sf.graph) {
    out << *sf.graph;
  } else {
    out << "Graph: Empty/Null" << std::endl;
  }
  out << std::endl
      << "-------------------------------------------------------------------"
      << std::endl;
  return out;
}
