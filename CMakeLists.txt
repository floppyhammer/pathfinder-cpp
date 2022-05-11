cmake_minimum_required(VERSION 3.16.1)
project(pathfinder)

set(CMAKE_CXX_STANDARD 17)

# Enable OpenMP.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp")

# Set binary output directory. Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

option(VULKAN_SUPPORT "Render with Vulkan instead of OpenGL")
set(VULKAN_SUPPORT ON)

if (VULKAN_SUPPORT)
    message("Using Vulkan")

    add_compile_definitions(PATHFINDER_USE_VULKAN)

    # More generally, you are using find_package and then later you're using add_subdirectory
    # to try to incorporate these external projects. That's not how it works. find_package will
    # look for a pre-existing binary of the package, while add_subdirectory isn't designed to
    # just swallow entire existing external CMake projects.
    # Cf. https://stackoverflow.com/questions/56795645/how-can-i-add-vulkan-to-a-cross-platform-cmake-project
    find_package(Vulkan REQUIRED FATAL_ERROR)

    # We need to include Vulkan headers manually because of the above reason.
    include_directories("C:/VulkanSDK/1.3.204.0/Include")
else ()
    message("Using OpenGL")
endif ()

# Include external and third_party headers.
include_directories(external third_party)

# GLFW library.
include_directories(external/glfw/include)
add_subdirectory(external/glfw)

# Core library.
add_subdirectory(src)

# Demo executable.
add_subdirectory(demo/native)
