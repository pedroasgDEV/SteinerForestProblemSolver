#include "../utils/graph.hpp"
#include "SFP.hpp"

bool isAllTerminalsPairsConnected(const Graph& solution, const std::vector<std::pair<int, int>>& terminals){
    for(auto terminal : terminals)
        if(!solution.isReachable(terminal.first, terminal.second)) return false;
    return true;
}
