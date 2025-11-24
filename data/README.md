# Test Instances

This directory contains a collection of benchmark instances for the **Steiner Forest Problem (SFP)**. The SFP, also known as the Prize-Collecting Steiner Tree Problem on graphs with terminal pairs, aims to find a minimum-cost subgraph that connects several specified pairs of terminal nodes (nets).

## Source and Citation

These instances were provided by Ghalami et al. and are associated with the following research paper. If you use these instances in your research, please cite:

> Ghalami, L., Gendron, B., & Gonzalez-Velarde, J. L. (2022).
> **An efficient formulation and branch-and-cut algorithm for the Steiner forest problem.**
> *Networks*, 79(4), 517–539.
> **DOI:** [`https://doi.org/10.1002/net.22046`](https://doi.org/10.1002/net.22046).

The original data repository can be found on GitHub:
[github.com/lalehghalami/SteinForestProblem](https://github.com/lalehghalami/SteinForestProblem)

---

## Directory Structure

The instances are organized into two main categories based on the graph type:

```

.
├── Sparse-Graphs
│   ├── Incidence
│   └── Random
└── WireRouting-Graphs
└── WRP3

```

* **Sparse-Graphs**: Contains sparsely connected graphs.
    * `Incidence`: Graphs derived from incidence matrices.
    * `Random`: Randomly generated sparse graphs.
* **WireRouting-Graphs**: Instances derived from VLSI wire routing problems.
    * `WRP3`: A specific benchmark set from this category.

---

## Instance File Format

Each instance file uses a plain text format divided into sections.

### `SECTION Graph`
This section defines the graph's structure.

* `Nodes <N>`: The total number of nodes in the graph (vertices).
* `Edges <M>`: The total number of edges.
* `E <u> <v> <cost>`: A line describing an undirected edge between node `u` and node `v` with an associated `cost`.

### `SECTION Terminals`
This section defines the terminal pairs (nets) that must be connected.

* `Terminals <K>`: The total number of terminal nodes (i.e., `K/2` pairs).
* `TP <s> <t>`: A terminal pair (net) that must be connected, consisting of node `s` and node `t`. The file will list `K/2` such lines.