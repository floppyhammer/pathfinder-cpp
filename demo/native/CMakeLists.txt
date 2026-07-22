# Build executable.
add_executable(pathfinder_demo_native main.cpp)

# Link libraries.
target_link_libraries(pathfinder_demo_native pathfinder_demo_common)

# Copy assets.
add_custom_command(TARGET pathfinder_demo_native POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)
