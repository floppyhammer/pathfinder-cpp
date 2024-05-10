# Build executable.
add_executable(pathfinder_demo_native ${SOURCE_FILES} main.cpp)

# Link libraries.
target_link_libraries(pathfinder_demo_native pathfinder_demo_common pathfinder)
