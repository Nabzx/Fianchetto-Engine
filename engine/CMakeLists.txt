cmake_minimum_required(VERSION 3.20)
project(FianchettoEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_NEURAL "Enable neural network evaluation via HTTP" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(ENGINE_SOURCES
    src/types.cpp
    src/board.cpp
    src/movegen.cpp
    src/search.cpp
)

if(USE_NEURAL)
    list(APPEND ENGINE_SOURCES src/neural_client.cpp)
endif()

# Create engine library
add_library(engine STATIC ${ENGINE_SOURCES})

# Link libraries
if(USE_NEURAL)
    find_package(CURL REQUIRED)
    target_link_libraries(engine PRIVATE CURL::libcurl)
    target_compile_definitions(engine PRIVATE USE_NEURAL)
endif()

# UCI executable
add_executable(fianchetto_uci
    src/uci_main.cpp
)
target_link_libraries(fianchetto_uci PRIVATE engine)

# Perft executable
add_executable(fianchetto_perft
    src/perft.cpp
)
target_link_libraries(fianchetto_perft PRIVATE engine)

# Tests
enable_testing()
find_package(Catch2 QUIET)
if(Catch2_FOUND)
    add_executable(fianchetto_tests
        tests/perft_tests.cpp
    )
    target_link_libraries(fianchetto_tests PRIVATE engine Catch2::Catch2)
    include(CTest)
    include(Catch)
    catch_discover_tests(fianchetto_tests)
endif()

