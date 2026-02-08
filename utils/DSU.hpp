#ifndef DSU_HPP
#define DSU_HPP

#include <numeric>
#include <vector>

/**
 * @struct DSU
 * @brief Optimized Disjoint Set Union (Union-Find) data structure.
 * * Implements the standard DSU with **Path Compression** and **Union by Rank**
 * optimizations. This structure is designed for high-performance connectivity
 * checks in the Steiner Forest Problem.
 * * @note The time complexity for operations is nearly constant, amortized
 * O(alpha(N)), where alpha is the inverse Ackermann function.
 */
struct DSU {
  std::vector<int> parent;  ///< Stores the immediate parent of each node.
  std::vector<int> rank;    ///< Stores the immediate parent of each node.
  int components;           ///< Tracks the current number of disjoint sets.

  /**
   * @brief Constructs the DSU and allocates memory.
   * @param nNodes Total number of nodes in the graph.
   */
  explicit DSU(int nNodes) {
    parent.resize(nNodes);
    rank.resize(nNodes);
    reset();
  }

  /**
   * @brief Resets the DSU to its initial state without reallocating memory.
   * * Sets every node as its own parent and resets ranks.
   */
  void reset() {
    std::iota(parent.begin(), parent.end(), 0);
    std::fill(rank.begin(), rank.end(), 0);
    components = parent.size();
  }

  /**
   * @brief Finds the representative (root) of the set containing element i.
   * @param i The element to search for.
   * @return int The index of the root element.
   */
  int find(int i) {
    if (parent[i] == i) return i;
    // Path Compression assignment
    return parent[i] = find(parent[i]);
  }

  /**
   * @brief Unites the sets containing source and target.
   * @param source First element.
   * @param target Second element.
   * @return true If the elements were in different sets and are now merged.
   * @return false If they were already in the same set.
   */
  bool unite(int source, int target) {
    int root_source = find(source);
    int root_target = find(target);

    if (root_source != root_target) {
      // Union by Rank logic
      if (rank[root_source] < rank[root_target]) {
        parent[root_source] = root_target;
      } else {
        parent[root_target] = root_source;
        if (rank[root_source] == rank[root_target]) {
          rank[root_source]++;
        }
      }
      components--;
      return true;
    }
    return false;
  }

  /**
   * @brief Checks if two elements belong to the same set.
   * @param source First element.
   * @param target Second element.
   * @return true If connected.
   * @return false If not connected.
   */
  bool isConnected(int source, int target) {
    return find(source) == find(target);
  }
};

#endif
