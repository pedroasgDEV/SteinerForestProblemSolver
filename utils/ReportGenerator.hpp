#ifndef REPORT_GENERATOR_HPP
#define REPORT_GENERATOR_HPP

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../algorithms/Solver.hpp"
#include "../models/SFP.hpp"

struct FileStats {
  std::string filename;
  size_t nNodes;
  size_t nEdges;
  size_t nTerminals;
  float originalCost;
  float solutionCost;
  float deltaCost;
  double timeMs;
  float alphaUsed;
};

/**
 * @brief Extracts the filename from a full directory path.
 * @param path The full path string.
 * @return string The extracted filename.
 */
std::string getFileName(const std::string& path);

/**
 * @brief Checks if a string ends with a specific suffix.
 * @param str The main string (e.g., filename).
 * @param suffix The ending string to check for.
 * @return true If 'str' ends with 'suffix', false if not.
 */
bool hasExtension(const std::string& str, const std::string& suffix);

/**
 * @brief Orchestrates the loading and solving of a single SFP instance file.
 * @param filepath Path to the input file.
 * @param alpha RCL parameter [0.0, 1.0] (greedy -> 0.0, random -> 1.0).
 * @return FileStats A struct containing the execution statistics.
 */
FileStats processFile(const std::string& filepath, const float alpha);

/**
 * @brief Orchestrates the loading and solving of a single SFP instance file
 * varying alpha from 0.0 to 1.0.
 * @param filepath Path to the input file.
 * @return FileStats A struct containing the execution statistics with the best
 * alpha value (min cost).
 */
FileStats findBestAlpha(const std::string& filepath);

/**
 * @brief Prints a formatted Markdown table row with the statistics of a single
 * file. Displays: Filename | Node Count | Terminal Count | Cost Ratio | Time
 * (ms).
 * @param s The statistics struct containing the data for the processed file.
 */
void printFileRow(const FileStats& s);

/**
 * @brief Generates and prints a Markdown summary report for a batch of
 * processed files.
 * @param sourceName Name of the directory or source identifier being
 * summarized.
 * @param stats A vector containing the statistics of all processed files.
 */
void printSummary(const std::string& sourceName,
                  const std::vector<FileStats>& stats);

/**
 * @brief Prints a Markdown table header.
 */
void printMarkdownHeader();

#endif
