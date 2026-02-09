# Steiner Forest Problem Solver

A C++ implementation of heuristic strategies for solving the **Steiner Forest Problem (SFP)**. This project focuses on constructive heuristics, refinement heuristics and metaheuristics to find low-cost solutions for connecting multiple terminal pairs in a graph.

-----
## Based on Research

This implementation is based on the work presented in the paper:

> **"Heurística GRASP para o Problema da Floresta de Steiner"** > *Proceedings of the LVII Simpósio Brasileiro de Pesquisa Operacional (SBPO 2025)* > [Read the full paper here](https://proceedings.science/sbpo/sbpo-2025/trabalhos/heuristica-grasp-para-o-problema-da-floresta-de-steiner?lang=pt-br)

-----
## Features

## Overview

The solver employs a two-phase metaheuristic approach:
1.  **Constructive Phase:** Generates an initial feasible solution using a randomized greedy algorithm controlled by an alpha parameter (Restricted Candidate List).
2.  **Local Search Phase:** Refines the solution using a "Destroy and Repair" strategy, optimized with a pruning mechanism to remove unnecessary non-terminal leaf nodes.

## Key Features

* **GRASP Metaheuristic:** Implements a robust loop of construction and local search to escape local optima.
* **Advanced Local Search:**
    * **Destroy and Repair:** Iteratively removes edges and reconnects broken components using shortest paths (Dijkstra).
    * **Pruning:** Automatically identifies and removes dead branches (degree-1 non-terminal nodes) to strictly minimize solution cost.
    * **Efficiency:** Uses Disjoint Set Union (DSU) for fast connectivity queries and an incremental cost update system.
* **High Performance:** Written in modern C++17 with custom graph data structures and optimized memory management.
* **Benchmarking Suite:** Includes a report generator that produces Markdown-formatted tables comparing results across different alpha values and datasets.

-----
## Project Structure

```
.
├── algorithms/       # Heuristics
├── models/           # SFP Classes
├── utils/            # Graph, DSU, CLI parser, Report generator and Dijkstra
├── tests/            # Unit tests for all modules
├── data/             # Benchmark instances (.stp files)
├── main.cpp          # Entry point (CLI Interface)
└── CMakeLists.txt    # Build configuration
````

-----
## Building the Project

Requirements:
* C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
* Standard C++ libraries (no external dependencies required)
  * **Linux:** GCC (g++)
  * **macOS:** Clang (via Xcode Command Line Tools)
  * **Windows:** MSVC (Visual Studio) or MinGW

### Linux & macOS (Terminal)

```bash
mkdir build
cd build
cmake ..
make
````

### Windows (PowerShell or CMD)

If you are using **Visual Studio** compilers (MSVC), CMake will generate a solution file. The best way to build from the command line is:

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

*Note: If using MinGW, you can use `cmake -G "MinGW Makefiles" ..` and then `mingw32-make`.*

-----

## Usage

The executable `steiner_forest` provides a CLI interface for running tests or solving instances.

**Note on Executable Paths:**

  * **Linux/macOS:** The executable is usually `./steiner_forest` inside the build folder.
  * **Windows:** The executable is `steiner_forest.exe`. If built with Visual Studio, it is likely located inside a subfolder, e.g., `.\Release\steiner_forest.exe`.

### 1\. Solving a Single File

To solve a specific `.stp` file:

**Linux / macOS:**

```bash
./steiner_forest -f data/Sparse-Graphs/Random/b01.stp
```

**Windows:**

```powershell
.\Release\steiner_forest.exe -f data\Sparse-Graphs\Random\b01.stp
```

### 2\. Tuning Alpha

You can set the `alpha` parameter (randomness factor for the constructive heuristic).

  * `0.0` = Pure Greedy.
  * `1.0` = Pure Random.

**Linux / macOS:**

```bash
./steiner_forest -f data/b01.stp -a 0.5
```

**Windows:**

```powershell
.\Release\steiner_forest.exe -f data\b01.stp -a 0.5
```

### 3\. Alpha Variation Mode

Automatically runs the heuristic multiple times with alphas ranging from `0.0` to `1.0` (steps of 0.1) and reports the best result found.

**Linux / macOS:**

```bash
./steiner_forest -f data/b01.stp -v
```

**Windows:**

```powershell
.\Release\steiner_forest.exe -f data\b01.stp -v
```

### 4\. Batch Processing (Directories)

Recursively process all `.stp` files found in a directory and its subdirectories.

**Linux / macOS:**

```bash
./steiner_forest -d data/Sparse-Graphs/
```

**Windows:**

```powershell
.\Release\steiner_forest.exe -d data\Sparse-Graphs\
```

### 5\. Running Tests

To run the unit test suite to verify the graph structure, algorithms, and logic:

**Linux / macOS:**

```bash
./steiner_forest --test
```

**Windows:**

```powershell
.\Release\steiner_forest.exe --test
```

-----

## Output Format

The program outputs results in a Markdown-compatible table format, perfect for documentation logs:

| File | Nodes | Edges | Terms | Ratio | Delta | Time (ms) | Best Alpha |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| i080-214.stp |    80 |   700 |     8 |  0.0672 |    -14.00 |     2.81 |       1.0 |

  * **File:** `Instance filename.`
  * **Nodes:** `Number of nodes in the graph.`
  * **Edges:** `Number of edges in the graph.`
  * **Terms:** `Number of terminal pairs.`
  * **Ratio:** `Solution Cost / Original Graph Total Cost` (Lower is bestter/sparser).
  * **Delta:** `Local Search Solution Cost - Constructive Solution Cost.`
  * **Time:** `Time in milliseconds it took to find the solution.`
  * **Best Alpha:** `The value of alpha that yielded the best solution.`
-----

## Third-Party Libraries & Resources

This project makes use of the following open-source libraries and resources:

* **[CLI11](https://github.com/CLIUtils/CLI11)**: A command line parser for C++11 and beyond.
    * **License**: [3-Clause BSD License](https://github.com/CLIUtils/CLI11/blob/main/LICENSE)
    * **Copyright**: © 2017-2024 University of Cincinnati.

* **[SteinForestProblem Instances](https://github.com/lalehghalami/SteinForestProblem)**: Benchmark datasets used for validating and testing the algorithms.
    * **Reference**: Ghalami, L., & Grosu, D. (2022). "Approximation algorithms for Steiner forest: An experimental study". *Networks*, 79(2), 164-188.
    * **DOI**: [10.1002/net.22046](https://doi.org/10.1002/net.22046)
