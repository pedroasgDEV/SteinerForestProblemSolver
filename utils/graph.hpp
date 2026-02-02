#ifndef GRAPH_HPP
#define GRAPH_HPP               

#include <ostream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <queue>
#include <tuple>

/**
 * @brief Represents an edge from the graph
*/
struct Edge {
    int target;
    float weight;
    bool active;
};

/**
 * @class Graph
 * @brief Class defining a graph to SFP.
 */
class Graph {
protected:
    std::vector<int> ptrs; ///< CSR row pointers. ptrs[i] marks the start of vertex i's edges in the edges vector. 
    std::vector<Edge> edges; ///< Contiguous array containing all graph edges, grouped by source vertex.
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
    Graph(const std::vector<std::tuple<int, int, float>>& edgeList, const int nNodes, const bool isBidirectional = true): 
        nNodes(nNodes), isBidirectional(isBidirectional), totalWeight(0.0f) 
    {
        if (nNodes <= 0) throw std::runtime_error("ERROR: Number of nodes must be positive.");
        if (edgeList.empty()) throw std::runtime_error("ERROR: edgeList cannot be empty");

        // Start with a temporary Adjacent List
        std::vector<std::vector<Edge>> adj(nNodes);
        for (const auto& edge : edgeList) {
            int origin = std::get<0>(edge);
            int target = std::get<1>(edge);
            float weight = std::get<2>(edge);

            if (origin < 0 || origin >= nNodes || target < 0 || target >= nNodes)
                throw std::runtime_error("ERROR: Edge index out of bounds.");

            adj[origin].push_back({target, weight, true});
            totalWeight += weight;
            if (isBidirectional) adj[target].push_back({origin, weight, true});
        }

        // Flattening to CSR 
        ptrs.reserve(nNodes + 1);
        nEdges = edgeList.size() * (isBidirectional ? 2 : 1); 
        edges.reserve(nEdges);

        ptrs.push_back(0);
        for (const auto& neighbors : adj) {
            for (const auto& edge : neighbors) edges.push_back(edge);
            ptrs.push_back(edges.size());
        }
    }
    
    // The empty default constructor should be deleted to avoid invalid graphs.
    Graph() = delete;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Graph () = default;

    /**
     * @brief Get the verticesEdgsPtr of the graph. 
     * @return Int Vector.
     */
    const std::vector<int>& getPtrs () const { return ptrs; };
    
    /**
     * @brief Get the edges vector of the graph. 
     * @return Edge Vector.
     */
    const std::vector<Edge>& getEdges () const { return edges; };
     
    /**
     * @brief Get the sum of all weight of the graph
     * @return Float value;
     */
    float getTotalWeight () const { return totalWeight; };

    /**
     * @brief Get If the graph is bidirectional. 
     * @return Bool value;
     */
    bool getIsBidirectional () const { return isBidirectional; };
    
    /**
     * @brief Get the number of nodes. 
     * @return int value;
     */
    int getNNode () const { return nNodes; };
 
    /**
     * @brief Get the number of edges. 
     * @return int value;
     */
    int getNEdges () const { return nEdges; };

    /**
     * @brief Turn all edges of the graph as non-active inactive
     */
    void inactiveAllEdges() { totalWeight = 0; for (auto edge : edges) edge.active = false; }

    /**
     * @brief active a edge between two vertices.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     */
    void activeEdge (const int from_id, const int to_id) {
        int edge_id = getEdge(from_id, to_id);
        
        if(edge_id == -1) return;

        if(!edges[edge_id].active){
            edges[edge_id].active = true;
            totalWeight += edges[edge_id].weight;
            
            try { if(isBidirectional) edges[getEdge(to_id, from_id)].active = true; }
            catch (...) { throw std::runtime_error("ERROR: the graph isn't bidirectional"); }
        }
    };  

    /**
     * @brief inactive a edge between two vertices.
     * @param from_id Int ID of the first vertex.
     * @param to_id INt ID of the second vertex.
     */
    void inactiveEdge (const int from_id, const int to_id) {
        int edge_id = getEdge(from_id, to_id);
        
        if(edge_id == -1) return;

        if(edges[edge_id].active){
            edges[edge_id].active = false;
            totalWeight -= edges[edge_id].weight;
            
            try { if(isBidirectional) edges[getEdge(to_id, from_id)].active = false; }
            catch (...) { throw std::runtime_error("ERROR: the graph isn't bidirectional"); }
        }
    };

    /**
     * @brief Checks whether vertex v_id is reachable from u_id with BFS.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     * @return Bool if the second vertex is reachable from the first.
     */
    bool isReachable (const int from_id, const int to_id) const {
        // Verify if the vertices exists 
        if(from_id >= nNodes || from_id < 0) throw std::runtime_error("ERROR: The Node from_id does not exist in this graph.");
        if(to_id >= nNodes || to_id < 0) throw std::runtime_error("ERROR: The Node to_id does not exist in this graph.");
        
        if(from_id == to_id) return true;

        // Vector to mark visited nodes and prevent infinite loops.
        std::vector<bool> visited(nNodes, false);
        
        // Queue for processing the nodes
        std::queue<int> q;

        visited[from_id] = true;
        q.push(from_id);

        while (!q.empty()) {
            int u = q.front();
            int u_ptr = ptrs[u];
            int nxt_ptr = ptrs[u + 1];
            q.pop();

            if (u == to_id) return true;

            for (int crnt_edge = u_ptr; crnt_edge < nxt_ptr; crnt_edge++)
                if (edges[crnt_edge].active && !visited[edges[crnt_edge].target]) {
                    visited[edges[crnt_edge].target] = true;
                    q.push(edges[crnt_edge].target);
                }
        }

        return false;

    };

    /**
     * @brief get the edge position between from_id and to_id.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     * @return int edge position (If returns -1, the edge does'n exist.) 
     */
    int getEdge (const int from_id, const int to_id) const {
        // Verify if the vertices exists 
        if(from_id >= nNodes || from_id < 0) throw std::runtime_error("ERROR: The Node from_id does not exist in this graph.");
        if(to_id >= nNodes || to_id < 0) throw std::runtime_error("ERROR: The Node to_id does not exist in this graph."); 
        
        for (int crnt_edge = ptrs[from_id]; crnt_edge < ptrs[from_id + 1]; crnt_edge++)
            if(edges[crnt_edge].target == to_id) return crnt_edge;
        
        return -1;
    };

    /**
     * @brief Overloads the << operator to print the graph.
     * @param out The output stream.
     * @param g The graph to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& out, const Graph& g) {
        out << std::endl << "## Graph implemented as Contiguous Adjacency Lists and CSR" << std::endl;
        out << "Total Weight: " << g.totalWeight << std::endl;
        out << "Is Bidirectional: " << ((g.isBidirectional) ? "True" : "False") << std::endl;

        out << std::endl << "### Edges of eatch Node" << std::endl;
        for (int crnt_node = 0; crnt_node < g.nNodes; crnt_node++ ){
            out << "Node " << crnt_node<< " ->";
            
            for (int crnt_edge = g.ptrs[crnt_node]; crnt_edge < g.ptrs[crnt_node + 1]; crnt_edge++){
                if(!g.edges[crnt_edge].active) continue;
                out << " {" << "Target " << g.edges[crnt_edge].target << ", Weight " << g.edges[crnt_edge].weight << "}";
            }

            out << ";" << std::endl; 
        } 

        return out;
    };
};

//---------------- Graphs Constraint Functions ----------------

/**
 * @brief Checks if the graph contains only non-negative weights.
 * @param G a Graph obj
 * @return true if the graph has a negative weight, false if not.
 */
bool hasNegativeWeights(const Graph& g);

/**
 * @brief Checks if the graph is connected using BFS.
 * @param G a Graph obj
 * @return true if the graph is connected, false if not.
 */
bool isGraphConnected(const Graph& g);

#endif
