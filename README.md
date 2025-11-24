# Steiner Forest Problem Solver

A C++ implementation of heuristic strategies for solving the **Steiner Forest Problem (SFP)**. This project focuses on constructive heuristics, refinement heuristics and metaheuristics to find low-cost solutions for connecting multiple terminal pairs in a graph.

-----
## Based on Research

This implementation is based on the work presented in the paper:

> **"Heurística GRASP para o Problema da Floresta de Steiner"** > *Proceedings of the LVII Simpósio Brasileiro de Pesquisa Operacional (SBPO 2025)* > [Read the full paper here](https://proceedings.science/sbpo/sbpo-2025/trabalhos/heuristica-grasp-para-o-problema-da-floresta-de-steiner?lang=pt-br)

-----
## Features

* **Graph Representation:** Adjacency matrix implementation optimized for dense operations.
* **Constructive Heuristic:** Shortest-path based greedy construction with dynamic cost updates.
* **File Support:** Parsing of standard `.stp` benchmark files.
* **Batch Processing:** Recursively process entire directories of instances.
* **Reporting:** Automatic Markdown table generation with solution costs, ratios, and execution times.
* **Cross-Platform:** Compatible with Linux, Windows, and macOS (C++11 Standard).

-----
## Project Structure

```
.
├── algorithms/       # Heuristics and other algorithms
├── models/           # SFP Class (Orchestrator for solving and verifying)
├── utils/            # Graph data structure, CLI parser, Report generator
├── tests/            # Unit tests for all modules
├── data/             # Benchmark instances (.stp files)
├── main.cpp          # Entry point (CLI Interface)
└── CMakeLists.txt    # Build configuration
````

-----
## Building the Project

Requirements:
* **CMake** (3.5 or higher)
* **C++ Compiler** supporting C++11:
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

| File | Nodes | Terms | Ratio | Time (ms) | Best Alpha |
| :--- | :---: | :---: | :---: | :---: | :---: |
| b01.stp | 50 | 5 | 0.4512 | 1.20 | 0.7 |

  * **File:** `Instance filename.`
  * **Nodes:** `Number of nodes in the graph.`
  * **Terms:** `Number of terminal pairs.`
  * **Ratio:** `Solution Cost / Original Graph Total Cost` (Lower is better/sparser).
  * **Time:** `Time in milliseconds it took to find the solution.`
  * **Best Alpha:** `The value of alpha that yielded the best solution.`