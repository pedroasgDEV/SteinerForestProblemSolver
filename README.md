# Steiner Forest Problem Solver

A C++ implementation of heuristic strategies for solving the **Steiner Forest Problem (SFP)**. This project focuses on constructive heuristics, refinement heuristics, and advanced metaheuristics (GRASP, VNS, and AM-VNS) to find low-cost solutions for connecting multiple terminal pairs in a graph.

-----
## Based on Research

This implementation is based on the work presented in the paper:

> **"Heurística GRASP para o Problema da Floresta de Steiner"** 
> *Proceedings of the LVII Simpósio Brasileiro de Pesquisa Operacional (SBPO 2025)* 
> [Read the full paper here](https://proceedings.science/sbpo/sbpo-2025/trabalhos/heuristica-grasp-para-o-problema-da-floresta-de-steiner?lang=pt-br)

-----
## Overview

The solver employs a robust multi-phase metaheuristic approach:
1.  **Constructive Phase:** Generates an initial feasible solution using a randomized greedy algorithm controlled by an alpha parameter (Restricted Candidate List).
2.  **Local Search & VNS:** Refines the solution using a "Destroy and Repair" strategy. It leverages Variable Neighborhood Search (VNS) by increasing the destruction magnitude to escape local optima.
3.  **Adaptive Memory VNS (AM-VNS):** An advanced hybrid solver utilizing state partitioning (GOOD, BAD, UGLY edges) to systematically explore and intensify the search space.

## Key Features

* **Multiple Metaheuristics:** Run the solver using Pure GRASP, GRASP+VNS, or the state-of-the-art AM-VNS.
* **Advanced Local Search (Shock Wave Mechanism):** 
    * **Destroy and Repair:** Iteratively removes edges and reconnects broken components using shortest paths (Dijkstra).
    * **Cascading Disconnections:** Controlled by `delta` parameters, the algorithm creates localized "shock waves" to disconnect competing terminal pairs, allowing the topology to evolve fundamentally.
* **Efficiency:** Uses Disjoint Set Union (DSU) for fast connectivity queries and an incremental cost update system.
* **High Performance:** Written in modern C++17 with custom graph data structures, optimized memory management, and stochastic components.
* **IRACE Tuning Integration:** Built-in support to output minimal data specifically for automated parameter tuning via the IRACE package.

-----
## Project Structure

.
├── algorithms/       # Metaheuristics (GRASP, VNS, AM-VNS)
├── models/           # SFP Classes
├── utils/            # Graph, DSU, CLI parser, and Dijkstra engine
├── tests/            # Unit tests for all modules
├── data/             # Benchmark instances (.stp files)
├── main.cpp          # Entry point (CLI Interface)
└── CMakeLists.txt    # Build configuration
```

-----
## Building the Project

Requirements:
* C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
* Standard C++ libraries (no external dependencies required)

### Linux & macOS (Terminal)

```bash
mkdir build
cd build
cmake ..
make
```

### Windows (PowerShell or CMD)

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

-----

## Usage

The executable `steiner_forest` provides a CLI interface for running the different metaheuristics and tuning modes.

### 1. Solving with AM-VNS (Recommended)
Runs the Adaptive Memory Variable Neighborhood Search. You can control the `alpha` (constructive randomness), `delta1` (Diversification shockwave), `delta2` (Intensification shockwave), and the number of iterations.

```bash
./steiner_forest --AMVNS -f data/instance.stp -a 1.0 -d 3 -D 2 -i 100
```

### 2. Solving with standard GRASP or VNS
You can trigger the standard GRASP or the GRASP+VNS implementations. They use a single `-d` parameter for the Local Search phase.

```bash
# Pure GRASP (Constructive + Local Search)
./steiner_forest --GRASP -f data/instance.stp -a 1.0 -d 2 -i 50

# GRASP + VNS
./steiner_forest --VNS -f data/instance.stp -a 1.0 -d 3 -i 100
```

### 3. IRACE Tuning Mode
If you are performing parameter tuning using IRACE, append the `--IRACE` flag. This suppresses the visual execution summary and outputs only the final solution cost required by the IRACE target-runner.

```bash
./steiner_forest --IRACE --AMVNS -f data/instance.stp -a 1.0 -d 3 -D 2 -i 100
```

### 4. Running the Test Suite
To verify the graph structure, algorithms, DSU, and Dijkstra implementations:

```bash
# Run all tests
./steiner_forest --test

# Run specific modules
./steiner_forest --test-graph
./steiner_forest --test-DSU
./steiner_forest --test-dijkstra
./steiner_forest --test-sfp
```

-----

## Output Format

The standard execution outputs a clean summary in the terminal containing the instance details, the initial constructive cost, the final optimized cost, and the execution time:

```text
================ EXECUTION SUMMARY ================
Instance File:      i160-343.stp        
Nodes:              160                 
Edges:              2000                
Terminals:          34                  
Alpha Used:         1.00                
---------------------------------------------------
First Solution Cost: 450.5000            
Solution Cost:       412.3000            
Execution Time:      1450.250 ms         
===================================================
```

-----

## Third-Party Libraries & Resources

This project makes use of the following open-source libraries and resources:

* **[CLI11](https://github.com/CLIUtils/CLI11)**: A command line parser for C++11 and beyond.
    * **License**: [3-Clause BSD License](https://github.com/CLIUtils/CLI11/blob/main/LICENSE)
    * **Copyright**: © 2017-2024 University of Cincinnati.

* **[SteinForestProblem Instances](https://github.com/lalehghalami/SteinForestProblem)**: Benchmark datasets used for validating and testing the algorithms.
    * **Reference**: Ghalami, L., & Grosu, D. (2022). "Approximation algorithms for Steiner forest: An experimental study". *Networks*, 79(2), 164-188.
    * **DOI**: [10.1002/net.22046](https://doi.org/10.1002/net.22046)
```
