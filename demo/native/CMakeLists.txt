# Include Pathfinder headers.
include_directories(../../include)

# Build executable.
add_executable(pathfinder_demo_native ${SOURCE_FILES} main.cpp)

# Link libraries.
if (WIN32) # Windows
    target_link_libraries(pathfinder_demo_native glfw pathfinder pathfinder_demo_common)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder_demo_native Vulkan::Vulkan)
    endif ()
else () # Linux
    target_link_libraries(pathfinder_demo_native glfw pathfinder_demo_common pathfinder)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder_demo_native -lvulkan)
    else ()
        target_link_libraries(pathfinder_demo_native -lGL)
    endif ()
endif ()
