# Build executable.
add_executable(pathfinder_demo_native main.cpp)

# Link libraries.
target_link_libraries(pathfinder_demo_native pathfinder_demo_common pathfinder)
