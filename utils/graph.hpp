#ifndef GRAPH_HPP
#define GRAPH_HPP  

#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>    

/**
 * @class Graph
 * @brief Class defining a graph to SFP.
 */
class Graph {
protected:
    std::vector<std::vector<float>> matrix;
    size_t nNode;
    float totalWeight;
    bool isBidirectional;

public:
    /**
     * @brief Constructor with weights.
     * @param matrix The graph in float matrix format.
     * @param nNode Int number of vertices, default is 1.
     * @param isBidirectional Bool if the graph is bidirectional, default is true.
     */
    Graph (const std::vector<std::vector<float>>& matrix, const bool isBidirectional = true):
    matrix(matrix), nNode(matrix.size()), isBidirectional(isBidirectional) {
        totalWeight = 0.0f;
        for (int i = 0; i < nNode; ++i)
            for (int j = i + 1; j < nNode; ++j) totalWeight += matrix[i][j];
    };

    /**
     * @brief Default constructor.
     * @param nNode Int number of vertices, default is 1.
     * @param isBidirectional Bool if the graph is bidirectional, default is true.
     */
    Graph (const int nNode = 0, const bool isBidirectional = true): 
    matrix(std::vector<std::vector<float>>(nNode,  std::vector<float>(nNode, 0))),
    nNode(nNode), totalWeight(0.0f), isBidirectional(isBidirectional) {};

    /**
     * @brief Deep Copy Constructor.
     * @param other The graph to copy from.
     */
    Graph (const Graph& other) = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Graph () = default;

    /**
     * @brief Get a matrix of weights of the graph. 
     * @return Matrix of float.
     */
    std::vector<std::vector<float>> getMatrix () const { return matrix; };

    /**
     * @brief Get the number of vertices of the graph. 
     * @return Int value;
     */
    size_t getNNode () const { return nNode; };

    /**
     * @brief Get the sum of all weight of the graph
     * @return Float value;
     */
    float getTotalWeight () const { return totalWeight; };

    /**
     * @brief If the graph is bidirectional. 
     * @return Bool value;
     */
    bool getIsBidirectional () const { return isBidirectional; };

    /**
     * @brief Adds a edge between two vertices.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     * @param weight float Weight of the edge, default is 1.
     */
    void addEdge (const int from_id, const int to_id, const float weight = 1)  {
        // Verify if the vertices exists
        try{
            if(from_id > nNode || from_id < 0) throw from_id;
            if(to_id > nNode || to_id < 0) throw to_id;
        }
        catch (int id) {
            std::cout << "ERRO: Node " << id << "does not exist in this graph." << std::endl;
            return;
        }

        float last_weight = matrix[from_id][to_id]; // If already has a another edge
        if(isBidirectional){
            matrix[from_id][to_id] = weight;
            matrix[to_id][from_id] = weight;
        }
        else matrix[from_id][to_id] = weight;
        totalWeight += weight - last_weight; // Mod total weight of the graph
    };

    /**
     * @brief Removes a edge between two vertices.
     * @param from_id Int ID of the first vertex.
     * @param to_id INt ID of the second vertex.
     */
    void removeEdge (const int from_id, const int to_id) {
        // Verify if the vertices exists
        try{
            if(from_id > nNode || from_id < 0) throw from_id;
            if(to_id > nNode || to_id < 0) throw to_id;
        }
        catch (int id) {
            std::cout << "ERRO: Node " << id << "does not exist in this graph." << std::endl;
            return;
        }

        float weight = matrix[from_id][to_id]; // Get the weight of the edge
        if(isBidirectional){
            matrix[from_id][to_id] = 0.0f;
            matrix[to_id][from_id] = 0.0f;
        }
        else matrix[from_id][to_id] = 0.0f;
        totalWeight -= weight; // Mod total weight of the graph
    };

    /**
     * @brief Checks whether vertex v_id is reachable from u_id with BFS.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     * @return Bool if the second vertex is reachable from the first.
     */
    bool isReachable (const int from_id, const int to_id) const {
        // Verify if the vertices exists or are the same
        try{
            if(from_id > nNode || from_id < 0) throw from_id;
            if(to_id > nNode || to_id < 0) throw to_id;
        }
        catch (int id) {
            std::cout << "ERRO: Node " << id << "does not exist in this graph." << std::endl;
            return false;
        }

        // Vector to mark visited sites and prevent infinite loops.
        std::vector<bool> visited(nNode, false);
        
        // Queue for processing the nodes
        std::queue<int> q;

        visited[from_id] = true;
        q.push(from_id);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            if (u == to_id) return true;

            for (int v = 0; v < nNode; v++) {
                if (matrix[u][v] != 0.0f && !visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }

        return false;

    };

    /**
     * @brief Checks whether v_id is adjacent to u_id.
     * @param from_id Int ID of the first vertex.
     * @param to_id Int ID of the second vertex.
     * @return Bool if the vertices are adjacent.
     */
    bool isAdjacent (const int from_id, const int to_id) const {
        // Verify if the vertices exists or are the same
        try{
            if(from_id > nNode || from_id < 0) throw from_id;
            if(to_id > nNode || to_id < 0) throw to_id;
        }
        catch (int id) {
            std::cout << "ERRO: Node " << id << "does not exist in this graph." << std::endl;
            return false;
        }

        return matrix[from_id][to_id] != 0.0f;
    };

    /**
     * @brief Verify if the graphs are equals.
     * @return Bool if are equals.
     */
    bool operator == (const Graph& other) const{
        return (this->isBidirectional == other.isBidirectional &&
                this->nNode == other.nNode &&
                this->totalWeight == other.totalWeight &&
                this->matrix == other.matrix);
    };

    /**
     * @brief Verify if the graphs are not equals.
     * @return Bool if are not equals.
     */
    bool operator != (const Graph& other) const { return !(*this == other); }

    /**
     * @brief Deep Copy Assignment Operator.
     * @param other The graph to assign from.
     * @return Reference to this graph.
     */
    Graph& operator = (const Graph& other){
        if (this == &other) return *this;

        this->matrix = other.matrix;
        this->nNode = other.nNode;
        this->isBidirectional = other.isBidirectional;
        this->totalWeight = other.totalWeight;

        return *this;
    };

    /**
     * @brief Overloads the << operator to print the graph.
     * @param out The output stream.
     * @param g The graph to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& out, const Graph& g) {
        out << std::endl << "## Graph Adjacent Matrix" << std::endl;
        out << "   ";
        for (int j = 0; j < g.nNode; j++) {
            out << std::setw(6) << j + 1;
        }
        out << std::endl;

        for (int i = 0; i < g.nNode; i++) {
            out << std::setw(2) << i + 1 << ":";
            for (int j = 0; j < g.nNode; j++) {
                out << std::setw(6) << std::fixed << std::setprecision(1) << g.matrix[i][j];
            }
            out << std::endl;
        }

        return out;
    };
};

//---------------- Graphs Constraint Functions ----------------

/**
 * @brief Checks if the graph contains only non-negative weights.
 * @param G a Graph obj
 * @return true if the graph has only non-negative weights, false if not.
 */
bool hasNegativeWeights(const Graph& g);

/**
 * @brief Checks if the graph is connected using BFS.
 * @param G a Graph obj
 * @return true if the graph is connected, false if not.
 */
bool isGraphConnected(const Graph& g);

#endif