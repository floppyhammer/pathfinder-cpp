cmake_minimum_required(VERSION 3.16.1)
project(pathfinder)

set(CMAKE_CXX_STANDARD 17)

# Enable OpenMP.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp")

# Set binary output directory. Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Include external and third_party headers.
include_directories(external third_party)

# GLFW library.
include_directories(external/glfw/include) # Include headers.
add_subdirectory(external/glfw)

# Core library.
add_subdirectory(src)

# Demo executable.
add_subdirectory(demo/native)
