#include "ReportGenerator.hpp"

std::string getFileName(const std::string& path) {
  size_t lastSlash = path.find_last_of("/\\");
  if (lastSlash != std::string::npos) return path.substr(lastSlash + 1);
  return path;
}

bool hasExtension(const std::string& str, const std::string& suffix) {
  std::string tmp_str = getFileName(str);
  if (tmp_str.length() < suffix.length()) return false;
  return tmp_str.compare(tmp_str.length() - suffix.length(), suffix.length(),
                         suffix) == 0;
}

FileStats processFile(const std::string& filepath, const float alpha) {
  SFPProblem sfp;

  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filepath << std::endl;
    return {"", 0, 0, 0, 0, 0, 0, 0, alpha};
  }

  try {
    file >> sfp;
  } catch (const std::exception& e) {
    std::cerr << "Error parsing file: " << filepath << e.what() << std::endl;
    return {"", 0, 0, 0, 0, 0, 0, 0, alpha};
  }

  auto start = std::chrono::high_resolution_clock::now();

  GRASPConstructiveHeuristic constructive(alpha);
  SFPSolution solution = constructive.generate(sfp);
  // SFPSolution solution = sfp.random_solution();
  float firstCost = solution.getObjectiveValue();
  GRASPLocalSearch localSearch;
  localSearch.optimize(solution);

  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;

  FileStats stats;
  stats.filename = getFileName(filepath);
  stats.nNodes = sfp.getNNodes();
  stats.nEdges = sfp.getNEdges();
  stats.nTerminals = sfp.getTerminals().size();
  stats.originalCost = sfp.getGraphPtr()->totalWeight;
  stats.solutionCost = solution.getObjectiveValue();
  stats.deltaCost = stats.solutionCost - firstCost;
  stats.timeMs = duration.count();
  stats.alphaUsed = alpha;

  return stats;
}

FileStats findBestAlpha(const std::string& filepath) {
  FileStats bestStats;
  bestStats.solutionCost = std::numeric_limits<float>::max();
  bestStats.nNodes = 0;

  for (int i = 0; i <= 10; ++i) {
    float currentAlpha = i / 10.0f;
    FileStats current = processFile(filepath, currentAlpha);
    if (current.nNodes == 0 && current.originalCost == 0) return current;
    if (current.solutionCost < bestStats.solutionCost)
      bestStats = current;
    else if (current.solutionCost == bestStats.solutionCost)
      if (current.timeMs < bestStats.timeMs) bestStats = current;
  }

  return bestStats;
}

void printMarkdownHeader() {
  std::cout << "| File | Nodes | Edges | Terms | Ratio | Delta | Time (ms) | "
               "Best Alpha |"
            << std::endl;
  std::cout
      << "| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |"
      << std::endl;
}

void printFileRow(const FileStats& s) {
  float ratio = (s.originalCost > 0) ? (s.solutionCost / s.originalCost) : 0.0f;

  std::cout << "| " << std::left << std::setw(20) << s.filename << " | "
            << std::right << std::setw(5) << s.nNodes << " | " << std::setw(5)
            << s.nEdges << " | " << std::setw(5) << s.nTerminals << " | "
            << std::fixed << std::setprecision(4) << std::setw(7) << ratio
            << " | " << std::setprecision(2) << std::setw(9) << s.deltaCost
            << " |" << std::setprecision(2) << std::setw(9) << s.timeMs << " |"
            << std::setprecision(1) << std::setw(10) << s.alphaUsed << " |"
            << std::endl;
}

void printSummary(const std::string& sourceName,
                  const std::vector<FileStats>& stats) {
  if (stats.empty()) return;

  size_t minNodes = stats[0].nNodes;
  size_t maxNodes = stats[0].nNodes;
  float maxRatio = std::numeric_limits<float>::min();
  float minRatio = std::numeric_limits<float>::max();

  int count = 0;

  std::map<int, int> alphaWins;

  for (const auto& s : stats) {
    if (s.nNodes == 0) continue;
    if (s.nNodes < minNodes) minNodes = s.nNodes;
    if (s.nNodes > maxNodes) maxNodes = s.nNodes;

    float ratio =
        (s.originalCost > 0) ? (s.solutionCost / s.originalCost) : 0.0f;
    if (ratio > maxRatio) maxRatio = ratio;
    if (ratio < minRatio) minRatio = (ratio == 0) ? minRatio : ratio;

    int alphaKey = (int)std::round(s.alphaUsed * 10);
    alphaWins[alphaKey]++;

    count++;
  }

  int bestAlphaKey = -1;
  int maxWins = -1;
  for (auto const& a : alphaWins)
    if (a.second > maxWins) {
      maxWins = a.second;
      bestAlphaKey = a.first;
    }
  float bestAlphaGlobal = bestAlphaKey / 10.0f;

  std::cout << "\n\n### Summary Report" << std::endl;
  std::cout
      << "| Source | Count | Nodes | Max Ratio | Min Ratio | Most Freq Alpha |"
      << std::endl;
  std::cout << "| :--- | :---: | :---: | :---: | :---: | :---: |" << std::endl;
  std::cout << "| " << getFileName(sourceName) << " | " << count << " | "
            << minNodes << "-" << maxNodes << " | " << std::fixed
            << std::setprecision(4) << maxRatio << " | " << std::fixed
            << std::setprecision(4) << minRatio << " | " << std::setprecision(1)
            << bestAlphaGlobal << " (" << maxWins << " wins) |" << std::endl;
}
