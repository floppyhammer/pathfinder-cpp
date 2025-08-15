# Include Pathfinder headers.
include_directories(../../)

# Build executable.
add_executable(pathfinder_demo_web main.cpp)

# Link libraries.
target_link_libraries(pathfinder_demo_web pathfinder_demo_common pathfinder)
