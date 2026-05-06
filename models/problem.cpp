#include "SFP.hpp"

SFPProblem::SFPProblem(std::shared_ptr<Graph> g,
                       const std::vector<std::pair<int, int>>& terminals)
    : graph(g), terminals(terminals), instanceName("Manual"){
  if (!g) throw std::runtime_error("\tGraph pointer cannot be null");
  if (hasNegativeWeights(*g))
    throw std::runtime_error("\tGraph has negative weights.");
  if (!isGraphConnected(*g))
    throw std::runtime_error("\tGraph is not connected.");
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
        edgeList.push_back({source - 1, target - 1, weight});
      }
    }

    else if (readingTerminals) {
      if (token == "Terminals") {
        int nTerminals;
        in >> nTerminals;
        sf.terminals.reserve(nTerminals);
      } 
      else if (token == "TP") {
        int source, target;
        in >> source >> target;
        sf.terminals.push_back({source - 1, target - 1});
      }
    }
  }

  if (nNodes > 0 && !edgeList.empty()) {
    try {
      sf.graph = std::make_shared<Graph>(edgeList, nNodes);

      if (hasNegativeWeights(*sf.graph)) {
        throw std::runtime_error("\tGraph has negative weights.");
      }

      if (!isGraphConnected(*sf.graph)) {
        throw std::runtime_error("\tGraph is disconnected.");
      }

    }

    catch (const std::exception& e) {
      throw std::runtime_error("\tError constructing graph\n" + std::string(e.what()));
    }
  }

  else
    throw std::runtime_error("\tInvalid or empty STP file structure.");
  
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
