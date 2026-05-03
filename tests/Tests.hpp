#ifndef TESTS_HPP
#define TESTS_HPP

#include <cassert>
#include <sstream>

#include "../models/SFP.hpp"
#include "../utils/DSU.hpp"
#include "../utils/Dijkstra.hpp"
#include "../utils/Graph.hpp"

void graphTests();
void dijkstraTests();
void dsuTests();
void steinerForestTests();

#endif
