# Build executable.
add_executable(pathfinder_demo_native ${SOURCE_FILES} main.cpp)

# Link libraries.
if (WIN32)
    target_link_libraries(pathfinder_demo_native glfw pathfinder pathfinder_demo_common)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder_demo_native Vulkan::Vulkan)
    endif ()
elseif (LINUX)
    target_link_libraries(pathfinder_demo_native pathfinder_demo_common pathfinder glfw)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder_demo_native -lvulkan)
    else ()
        target_link_libraries(pathfinder_demo_native -lGL)
    endif ()
elseif (APPLE)
    target_link_libraries(pathfinder_demo_native glfw pathfinder_demo_common pathfinder Vulkan::Vulkan)
endif ()
