#include <iostream>
#include <string>
#include <vector>

#include "tests/tests.hpp"
#include "utils/CLI11.hpp"
// #include "utils/report_generator.hpp"
// #include "algorithms/Solver.hpp"
// #include "utils/Graph.hpp"
// #include "utils/DSU.hpp"
// #include "models/SFP.hpp"

#ifdef _WIN32
#include <windows.h>
const char PATH_SEP = '\\';

void getFilesInDirectory(const std::string& dirPath,
                         std::vector<std::string>& outFiles) {
  std::string searchPath = dirPath;
  if (searchPath.back() != PATH_SEP) searchPath += PATH_SEP;
  std::string wildcardPath = searchPath + "*.*";

  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(wildcardPath.c_str(), &fd);

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
        continue;

      std::string fullFileName = searchPath + fd.cFileName;

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        getFilesInDirectory(fullFileName, outFiles);
      else
        outFiles.push_back(fullFileName);
    } while (::FindNextFile(hFind, &fd));
    ::FindClose(hFind);
  }

#else
#include <dirent.h>
#include <sys/stat.h>
const char PATH_SEP = '/';

void getFilesInDirectory(const std::string& dirPath,
                         std::vector<std::string>& outFiles) {
  DIR* dir = opendir(dirPath.c_str());
  struct dirent* ent;

  if (dir != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      std::string fname = ent->d_name;
      if (fname == "." || fname == "..") continue;

      std::string fullPath = dirPath;
      if (fullPath.back() != PATH_SEP) fullPath += PATH_SEP;
      fullPath += fname;

      struct stat st;
      if (stat(fullPath.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode))
          getFilesInDirectory(fullPath, outFiles);
        else
          outFiles.push_back(fullPath);
      }
    }
    closedir(dir);
  } else {
    std::cerr << "ERROR: Could not open directory: " << dirPath << std::endl;
  }
}
#endif

  int main(int argc, char** argv) {
    CLI::App app{"Steiner Forest Problem Solver"};

    bool flag_test_all = false;
    bool flag_test_graph = false;
    bool flag_test_DSU = false;
    bool flag_test_dijkstra = false;
    bool flag_test_SFP = false;
    bool flag_test_GRASPcons = false;

    app.add_flag("--test", flag_test_all, "Runs all available tests");
    app.add_flag("--test-graph", flag_test_graph,
                 "Runs only the Graph struct tests");
    app.add_flag("--test-DSU", flag_test_DSU, "Runs only the DSU struct tests");
    app.add_flag("--test-dijkstra", flag_test_dijkstra,
                 "Runs only the Dijkstra algorithm tests");
    app.add_flag("--test-SFP", flag_test_SFP,
                 "Runs only the Steiner Forest Problem implementation tests");
    app.add_flag("--test-GRASPCONS", flag_test_GRASPcons,
                 "Runs only GRASP constructive heuristic tests");

    std::string input_file;
    std::string input_dir;
    float alpha = 1.0f;
    bool alpha_variation = false;

    app.add_option("-f,--file", input_file,
                   "Path to a single .stp file to solve")
        ->check(CLI::ExistingFile);
    app.add_option("-d,--directory", input_dir,
                   "Path to a directory containing .stp files (Without the "
                   "\"\\\" or \"/\" at the end)")
        ->check(CLI::ExistingDirectory);
    app.add_option("-a,--alpha", alpha,
                   "Alpha parameter for constructive heuristic")
        ->check(CLI::Range(0.0, 1.0));
    app.add_flag("-v,--variation", alpha_variation,
                 "Test alphas [0.0, 0.1 ... 1.0] and pick best");

    CLI11_PARSE(app, argc, argv);

    if (flag_test_all || flag_test_graph || flag_test_dijkstra ||
        flag_test_SFP || flag_test_GRASPcons) {
      if (flag_test_all) {
        graphTests();
        dijkstraTests();
        dsuTests();
        steinerForestTests();
        GRASPconstructiveTests();
        return 0;
      }
      if (flag_test_graph) graphTests();
      if (flag_test_dijkstra) dijkstraTests();
      if (flag_test_DSU) dsuTests();
      if (flag_test_SFP) steinerForestTests();
      if (flag_test_GRASPcons) GRASPconstructiveTests();
      return 0;
    }
    /*
    if (!input_file.empty()) {
        if (hasExtension(input_file, ".stp")) {
            FileStats stats;

            if (alpha_variation)
                stats = findBestAlpha(input_file,
                                        GRASPconstructiveHeuristic,
                                        {hasNegativeWeights, isGraphConnected},
                                        isAllTerminalsPairsConnected);
            else
                stats = processFile(input_file,
                                    GRASPconstructiveHeuristic,
                                    {hasNegativeWeights, isGraphConnected},
                                    isAllTerminalsPairsConnected,
                                    alpha);

            printMarkdownHeader();
            printFileRow(stats);
        }
        else{
            std::cout << "ERROR: The file" << input_file << " is not \".stp\"";
        }

    }
    else if (!input_dir.empty()) {
        std::vector<std::string> files;
        std::vector<FileStats> results;

        getFilesInDirectory(input_dir, files);

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return 0;
        }

        printMarkdownHeader();

        for (const auto& fullPath : files) {
            if (hasExtension(fullPath, ".stp")) {

                FileStats stats;
                if (alpha_variation)
                    stats = findBestAlpha(fullPath,
                                            GRASPconstructiveHeuristic,
                                            {hasNegativeWeights,
    isGraphConnected}, isAllTerminalsPairsConnected); else stats =
    processFile(fullPath, GRASPconstructiveHeuristic, {hasNegativeWeights,
    isGraphConnected}, isAllTerminalsPairsConnected, alpha);

                if (stats.nNodes > 0) {
                    printFileRow(stats);
                    results.push_back(stats);
                }
            }
        }

        printSummary(input_dir, results);
    }
    else {
        std::cout << "No input provided. Use --help to see options." <<
    std::endl;
    }
    */
    return 0;
  }
