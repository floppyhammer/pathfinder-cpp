cmake_minimum_required(VERSION 3.16.1)
project(pathfinder)

set(CMAKE_CXX_STANDARD 17)

# Set binary output directory. Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Reset CMake cache for these to take effect.
option(VULKAN_SUPPORT "Vulkan over OpenGL" ON)
option(BUILD_DEMO "Build demo" OFF)

if (VULKAN_SUPPORT)
    message("Will use Vulkan")

    add_compile_definitions(PATHFINDER_USE_VULKAN)

    # More generally, you are using find_package and then later you're using add_subdirectory
    # to try to incorporate these external projects. That's not how it works. find_package will
    # look for a pre-existing binary of the package, while add_subdirectory isn't designed to
    # just swallow entire existing external CMake projects.
    # Cf. https://stackoverflow.com/questions/56795645/how-can-i-add-vulkan-to-a-cross-platform-cmake-project
    find_package(Vulkan REQUIRED)

    # We need to include Vulkan headers manually because of the above reason.
    include_directories(${Vulkan_INCLUDE_DIRS})
else ()
    message("Will use OpenGL")
endif ()

# Include external and third_party headers.
include_directories(external third_party)

# GLFW library.
include_directories(external/glfw/include)

# Core library.
add_subdirectory(src)

if (BUILD_DEMO)
    message("Will build demo")

    # Link GLFW.
    add_subdirectory(external/glfw)

    # Demo executable.
    add_subdirectory(demo/native)
endif ()
