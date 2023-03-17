# Include Pathfinder headers.
include_directories(../../include)

# Build executable.
add_executable(native-demo ${SOURCE_FILES} main.cpp)

# Link libraries.
if (WIN32) # Windows
    target_link_libraries(native-demo glfw pathfinder pathfinder_demo_common)
    if (VULKAN_SUPPORT)
        target_link_libraries(native-demo Vulkan::Vulkan)
    endif ()
else () # Linux
    # sudo apt install libglfw3-dev
    # -lglfw for glfw
    # -ldl for glad
    # -lGL for opengl
    target_link_libraries(native-demo -lglfw -ldl -lGL -lpathfinder -lpathfinder_demo_common)
    if (VULKAN_SUPPORT)
        target_link_libraries(native-demo -lvulkan)
    endif ()
endif ()
