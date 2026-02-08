#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <ostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <queue>

/**
 * @struct Edge
 * @brief Represents an edge from the graph.
 * Optimized for SFP Metaheuristic: contains direct pointers to source and
 * reverse edge.
 */
struct Edge {
  int source;          ///< ID of the source node
  int target;          ///< ID of the target node
  int reverseEdgePtr;  ///< Index of the reverse edge. -1 if is not alocated yet.
  float weight;        ///< The weight of this edge
};

/**
 * @struct Graph
 * @brief Struct defining a graph to SFP using CSR (Compressed Sparse Row).
 */
struct Graph {
  std::vector<int> ptrs;    ///< CSR row pointers.
  std::vector<Edge> edges;  ///< Contiguous array containing all graph edges.
  const float totalWeight;  ///< The sum of all weight of this graph
  const int nNodes, nEdges; ///< the number of nodes and edges of this graph
  
  /**
   * @brief Construtor Raw Data -> CSR.
   * @param nNodes Number of nodes.
   * @param edgeList tuple vector {origin:int, target:int, weight:float}.
   */
  Graph(const std::vector<std::tuple<int, int, float>>& edgeList,
        const int nNodes)
      : totalWeight(0.0f),
        nNodes(nNodes),
        nEdges(edgeList.size() * 2)
    {
    if (nNodes <= 0)
      throw std::runtime_error("ERROR: Number of nodes must be positive.");
    if (edgeList.empty())
      throw std::runtime_error("ERROR: edgeList cannot be empty");

    // Temporary Adjacency List
    struct TempEdge { int target; float weight; };
    std::vector<std::vector<TempEdge>> adj(nNodes);

    double tempTotalWeight = 0;

    for (const auto& edge : edgeList) {
      int origin = std::get<0>(edge);
      int target = std::get<1>(edge);
      float weight = std::get<2>(edge);

      if (origin < 0 || origin >= nNodes || target < 0 || target >= nNodes)
        throw std::runtime_error("ERROR: Edge index out of bounds.");

      adj[origin].push_back({target, weight});
      adj[target].push_back({origin, weight});
      tempTotalWeight += weight;
    }

    // Flattening to CSR
    ptrs.reserve(nNodes + 1);
    edges.reserve(nEdges);
    ptrs.push_back(0);

    for (int i = 0; i < nNodes; i++) {
      // Sorting makes finding reverse edges deterministic and faster
      std::sort(adj[i].begin(), adj[i].end(), 
        [](const TempEdge& a, const TempEdge& b){ return a.target < b.target; });
      // Initialize reverseEdgePtr to -1
      for (const auto& edge : adj[i]) edges.push_back({i, edge.target, -1, edge.weight});
      ptrs.push_back(edges.size());
    }

    // Linking Reverse Edges
    for (int i = 0; i < nEdges; i++) {
      // Optimization: If already linked by its twin, skip
      if (edges[i].reverseEdgePtr != -1) continue;

      int source = edges[i].source;
      int target = edges[i].target;

      // Find the index of edge v -> u
      for (int j = ptrs[target]; j < ptrs[target+1]; ++j)
          if (edges[j].target == source) {
              // Link them both ways
              // const_cast needed because we are initializing 'const' members in body
              const_cast<int&>(edges[i].reverseEdgePtr) = j;
              const_cast<int&>(edges[j].reverseEdgePtr) = i;
              break;
          }
      const_cast<float&>(totalWeight) = static_cast<float>(tempTotalWeight);
    }
  }

  // Delete Copy/Assignment to prevent accidental expensive copies
  Graph(const Graph&) = delete;
  Graph& operator=(const Graph&) = delete;


  /**
   * @brief Overloads the << operator to print the graph.
   * @param out The output stream.
   * @param g The graph to print.
   * @return The output stream.
   */
  friend std::ostream& operator<<(std::ostream& out, const Graph& g) {
    out << std::endl << "-------------------------------------------------------------------" << std::endl;
    out << std::endl << "## Graph implemented as Contiguous Adjacency Lists and CSR" << std::endl;
    out << "Total Weight: " << g.totalWeight << std::endl;

    out << std::endl << "### Edges of eatch Node" << std::endl;
    for (int crnt_node = 0; crnt_node < g.nNodes; crnt_node++) {
      out << "Node " << crnt_node << " ->";

      for (int crnt_edge = g.ptrs[crnt_node]; crnt_edge < g.ptrs[crnt_node + 1]; crnt_edge++)
        out << " {" << "Target " << g.edges[crnt_edge].target << ", Weight " << g.edges[crnt_edge].weight << "}";

      out << ";" << std::endl;
    }
    out << std::endl << "-------------------------------------------------------------------"<< std::endl;

    return out;
  }
};

//---------------- Helper Functions ----------------

/**
 * @brief Checks if the graph contains only non-negative weights.
 * @param g a Graph obj
 * @return true if the graph has a negative weight, false if not.
 */
inline bool hasNegativeWeights(const Graph& g) {
  for (const auto& edge : g.edges)
    if (edge.weight < 0) return true;
  return false;
}

/**
 * @brief Checks if the graph is connected using BFS.
 * @param g a Graph obj.
 * @return true if the graph is connected, false if not.
 */
inline bool isGraphConnected(const Graph& g) {
  int count = 0;
  std::vector<bool> visited(g.nNodes, false);
  std::queue<int> q;

  visited[0] = true;
  q.push(0);
  count++;

  const auto& ptr = g.ptrs;
  const auto& edges = g.edges;

  while (!q.empty()) {
    int u = q.front();
    q.pop();

    for (int i = ptr[u]; i < ptr[u + 1]; ++i)
      if (!visited[edges[i].target]) {
        visited[edges[i].target] = true;
        count++;
        q.push(edges[i].target);
      }
  }

  return count == g.nNodes;
}

#endif
