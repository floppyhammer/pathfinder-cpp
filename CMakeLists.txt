cmake_minimum_required(VERSION 3.16.1)

project(pathfinder)

set(CMAKE_CXX_STANDARD 11)

# Set binary output directory.
# Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

option(VULKAN_SUPPORT "Use Vulkan instead of OpenGL")
option(BUILD_DEMO "Build demo")
option(BUILD_FOR_WEB "Build for web")

set(VULKAN_SUPPORT ON)
set(BUILD_DEMO OFF)
set(BUILD_FOR_WEB OFF)

# No Vulkan support for web.
if (BUILD_FOR_WEB)
    message("Will build for web")
    set(VULKAN_SUPPORT OFF)
else ()
    # Include GLFW headers.
    # Emscripten has its own version of GLFW.
    include_directories(external/glfw/include)
endif ()

if (VULKAN_SUPPORT)
    message("Will use Vulkan")

    add_compile_definitions(PATHFINDER_USE_VULKAN)

    # In case Vulkan is found but Vulkan_LIBRARY is not.
    #    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.224.1/Lib/vulkan-1.lib")

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

# Core library.
add_subdirectory(src)

# Add demo.
if (BUILD_DEMO)
    message("Will build demo")

    if (BUILD_FOR_WEB)
        # Set output directory to the web demo, so we don't have to copy output files manually.
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo/web")

        string(APPEND CMAKE_CXX_FLAGS " -s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1")
        string(APPEND CMAKE_CXX_FLAGS " -s WASM_BIGINT")

        # For debugging.
        string(APPEND CMAKE_CXX_FLAGS " -gsource-map -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1 --source-map-base http://127.0.0.1:8000/")

        add_subdirectory(demo/web)
    else ()
        add_subdirectory(external/glfw)

        add_subdirectory(demo/native)
    endif ()
endif ()
