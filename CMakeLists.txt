cmake_minimum_required(VERSION 3.10)
project(SteinerForestSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

set(SOURCES
    "main.cpp"

    "algorithms/constructive.cpp"
    "algorithms/localSearch.cpp"
    
    "models/problem.cpp"
    "models/solution.cpp"
    "models/neighbourhood.cpp"

    "utils/ReportGenerator.cpp"

    "tests/dsu.cpp"    
    "tests/constructive.cpp"    
    "tests/localSearch.cpp"    
    "tests/graph.cpp"
    "tests/dijkstra.cpp"
    "tests/sfp.cpp"
)

add_executable(steiner_forest ${SOURCES})

target_include_directories(steiner_forest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/algorithms
    ${CMAKE_SOURCE_DIR}/models
)

if(MSVC)
    target_compile_options(steiner_forest PRIVATE /W4)
else()
    target_compile_options(steiner_forest PRIVATE -Wall -Wextra -pedantic)
    endif()
