cmake_minimum_required(VERSION 3.10)
project(SteinerForestSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

# --- Seleção Manual de Arquivos ---
# Listamos apenas os arquivos .cpp necessários para o build.
# Note: .h e .hpp não precisam estar aqui para compilar, 
# mas incluí-los ajuda algumas IDEs a mostrá-los no projeto.
set(SOURCES
    "main.cpp"
    
    # Utils
    "utils/graph.cpp"
    
    # Algorithms
    "algorithms/dijkstra.cpp"
    
    "tests/graph.cpp"
    "tests/dijkstra.cpp"
)
# Criar o executável apenas com os arquivos selecionados
add_executable(steiner_forest ${SOURCES})

# --- Configuração de Cabeçalhos ---
# O CMake precisa saber onde procurar os arquivos .h e .hpp
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
