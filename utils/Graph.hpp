#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <iostream>
#include <ostream>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <vector>

/**
 * @struct Edge
 * @brief Represents an edge from the graph.
 * Optimized for SFP Metaheuristic: contains direct pointers to source and
 * reverse edge.
 */
struct Edge {
  int source;          ///< ID of the source node
  int target;          ///< ID of the target node
  int reverseEdgePtr;  ///< Index of the reverse edge. -1 if is not
                       ///< bidirectional.
  float weight;
  bool active;
};

/**
 * @class Graph
 * @brief Class defining a graph to SFP using CSR (Compressed Sparse Row).
 */
class Graph {
 private:
  std::vector<int> ptrs;  ///< CSR row pointers. ptrs[i] marks the start of node
                          ///< i's edges in the edges vector.
  std::vector<Edge> edges;  ///< Contiguous array containing all graph edges.
  float totalWeight;
  bool isBidirectional;
  int nNodes, nEdges;

 public:
  /**
   * @brief Construtor Raw Data -> CSR.
   * @param nNodes Number of nodes.
   * @param edgeList tuple vector {origin:int, target:int, weight:float}.
   * @param bidirectional if true create returns edges.
   */
  Graph(const std::vector<std::tuple<int, int, float>>& edgeList,
        const int nNodes, const bool isBidirectional = true)
      : totalWeight(0.0f), isBidirectional(isBidirectional), nNodes(nNodes) {
    if (nNodes <= 0)
      throw std::runtime_error("ERROR: Number of nodes must be positive.");
    if (edgeList.empty())
      throw std::runtime_error("ERROR: edgeList cannot be empty");

    // Temporary Adjacency List
    struct TempEdge {
      int target;
      float weight;
    };
    std::vector<std::vector<TempEdge>> adj(nNodes);

    for (const auto& edge : edgeList) {
      int origin = std::get<0>(edge);
      int target = std::get<1>(edge);
      float weight = std::get<2>(edge);

      if (origin < 0 || origin >= nNodes || target < 0 || target >= nNodes)
        throw std::runtime_error("ERROR: Edge index out of bounds.");

      adj[origin].push_back({target, weight});
      totalWeight += weight;
      if (isBidirectional) adj[target].push_back({origin, weight});
    }

    // Flattening to CSR
    ptrs.reserve(nNodes + 1);
    nEdges = edgeList.size() * (isBidirectional ? 2 : 1);
    edges.reserve(nEdges);

    ptrs.push_back(0);
    for (int source = 0; source < nNodes; source++) {
      // Initialize reverseEdgePtr to -1
      for (const auto& edge : adj[source])
        edges.push_back({source, edge.target, -1, edge.weight, true});
      ptrs.push_back(edges.size());
    }

    // Linking Reverse Edges
    if (isBidirectional) {
      for (int i = 0; i < nEdges; i++) {
        // Optimization: If already linked by its twin, skip
        if (edges[i].reverseEdgePtr != -1) continue;

        int source = edges[i].source;
        int target = edges[i].target;

        // Find the index of edge v -> u
        int revIdx = getEdge(target, source);

        if (revIdx != -1) {
          edges[i].reverseEdgePtr = revIdx;
          edges[revIdx].reverseEdgePtr = i;
        }
      }
    }
  }

  Graph() = delete;  // The empty default constructor should be deleted to avoid
                     // invalid graphs.
  virtual ~Graph() = default;

  // --- Getters ---
  const std::vector<int>& getPtrs() const { return ptrs; };
  const std::vector<Edge>& getEdges() const { return edges; };
  float getTotalWeight() const { return totalWeight; };
  bool getIsBidirectional() const { return isBidirectional; };
  int getNNodes() const { return nNodes; };
  int getNEdges() const { return nEdges; };

  /**
   * @brief Turns all edges inactive.
   * @param status True to activate, False to deactivate.
   */
  void setAllEdgesStatus(const bool status) {
    totalWeight = 0;
    for (auto& edge : edges) {
      if (status == true) totalWeight += edge.weight;
      edge.active = status;
    }
  }

  /**
   * @brief Sets the active status of an edge by its direct INDEX.
   * This is the O(1) method used by the Metaheuristic.
   * @param edgeIdx Index in the edges vector.
   * @param status True to activate, False to deactivate.
   */
  void setEdgeStatus(int edgeIdx, bool status) {
    if (edgeIdx < 0 || edgeIdx >= nEdges)
      throw std::runtime_error("ERROR: Edge out of bounds.");

    Edge& temp = edges[edgeIdx];

    if (temp.active == status) return;
    temp.active = status;

    if (status)
      totalWeight += temp.weight;
    else
      totalWeight -= temp.weight;

    if (isBidirectional) edges[temp.reverseEdgePtr].active = status;
  }

  /**
   * @brief Checks whether node target is reachable from source with BFS.
   * @param source Int ID of the first node.
   * @param target Int ID of the second node.
   * @return Bool if the second node is reachable from the first.
   * Optimized with Lazy Reset (Token) and Static Memory.
   */
  bool isReachable(const int source, const int target) const {
    if (source >= nNodes || source < 0)
      throw std::runtime_error("ERROR: Node from_id out of bounds.");
    if (target >= nNodes || target < 0)
      throw std::runtime_error("EROOR: Node to_id out of bounds.");

    if (source == target) return true;

    //! Warning: This makes the method Thread-Unsafe. I will change it in the
    //! next version to add parallism in Metaheuristic algs.
    static std::vector<int> visitedToken;
    static std::vector<int> q;  // Vector used as a Queue
    static int currentToken = 0;

    // Resize only if the graph grew
    if ((int)visitedToken.size() < nNodes) {
      visitedToken.resize(nNodes, 0);
      q.resize(nNodes);
    }

    currentToken++;
    int head = 0;
    int tail = 0;
    q[tail++] = source;
    visitedToken[source] = currentToken;

    while (head < tail) {
      int u = q[head++];

      if (u == target) return true;

      for (int i = ptrs[u]; i < ptrs[u + 1]; i++) {
        const auto& edge = edges[i];

        if (edge.active) {
          int v = edge.target;

          if (visitedToken[v] != currentToken) {
            visitedToken[v] = currentToken;
            q[tail++] = v;
          }
        }
      }
    }
    return false;
  }

  /**
   * @brief get the edge position between source and target.
   * @param source Int ID of the first node.
   * @param target Int ID of the second node.
   * @return int edge position (If returns -1, the edge does'n exist.).
   */
  int getEdge(const int source, const int target) const {
    // Verify if the vertices exists
    if (source >= nNodes || source < 0)
      throw std::runtime_error(
          "ERROR: The Node from_id does not exist in this graph.");
    if (target >= nNodes || target < 0)
      throw std::runtime_error(
          "ERROR: The Node to_id does not exist in this graph.");

    for (int i = ptrs[source]; i < ptrs[source + 1]; i++)
      if (edges[i].target == target) return i;

    return -1;
  }

  /**
   * @brief Overloads the << operator to print the graph.
   * @param out The output stream.
   * @param g The graph to print.
   * @return The output stream.
   */
  friend std::ostream& operator<<(std::ostream& out, const Graph& g) {
    out << std::endl
        << "-------------------------------------------------------------------"
           "----------"
        << std::endl;
    out << std::endl
        << "## Graph implemented as Contiguous Adjacency Lists and CSR"
        << std::endl;
    out << "Total Weight: " << g.totalWeight << std::endl;
    out << "Is Bidirectional: " << ((g.isBidirectional) ? "True" : "False")
        << std::endl;

    out << std::endl << "### Edges of eatch Node" << std::endl;
    for (int crnt_node = 0; crnt_node < g.nNodes; crnt_node++) {
      out << "Node " << crnt_node << " ->";

      for (int crnt_edge = g.ptrs[crnt_node]; crnt_edge < g.ptrs[crnt_node + 1];
           crnt_edge++) {
        if (!g.edges[crnt_edge].active) continue;
        out << " {" << "Target " << g.edges[crnt_edge].target << ", Weight "
            << g.edges[crnt_edge].weight << "}";
      }

      out << ";" << std::endl;
    }
    out << std::endl
        << "-------------------------------------------------------------------"
           "----------"
        << std::endl;

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
  for (const auto& edge : g.getEdges())
    if (edge.active && edge.weight < 0) return true;
  return false;
}

/**
 * @brief Checks if the graph is connected using BFS.
 * @param g a Graph obj.
 * @return true if the graph is connected, false if not.
 */
inline bool isGraphConnected(const Graph& g) {
  int count = 0;
  std::vector<bool> visited(g.getNNodes(), false);
  std::queue<int> q;

  visited[0] = true;
  q.push(0);
  count++;

  const auto& ptr = g.getPtrs();
  const auto& edges = g.getEdges();

  while (!q.empty()) {
    int u = q.front();
    q.pop();

    for (int i = ptr[u]; i < ptr[u + 1]; ++i)
      if (edges[i].active && !visited[edges[i].target]) {
        visited[edges[i].target] = true;
        count++;
        q.push(edges[i].target);
      }
  }

  return count == g.getNNodes();
}

#endif
