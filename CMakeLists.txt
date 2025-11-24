cmake_minimum_required(VERSION 3.5)
project(SteinerForestSolver)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED False)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

file(GLOB SOURCES 
    "main.cpp"         
    "tests/*.cpp"
    "algorithms/*.cpp"   
    "utils/*.cpp"   
    "models/*.cpp"   
)

add_executable(steiner_forest ${SOURCES})


include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/algorithms
    ${CMAKE_SOURCE_DIR}/models
)