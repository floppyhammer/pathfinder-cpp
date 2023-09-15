cmake_minimum_required(VERSION 3.16.1)

project(pathfinder)

set(CMAKE_CXX_STANDARD 14)

# Set binary output directory.
# Using CMAKE_CURRENT_SOURCE_DIR is necessary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Preset.
option(PATHFINDER_VULKAN_SUPPORT "Use Vulkan instead of OpenGL" OFF)
option(PATHFINDER_BUILD_DEMO "Build demo" OFF)

# Identify Linux.
if (UNIX AND NOT APPLE)
    set(LINUX ON)
endif ()

# No GL support for Mac.
if (APPLE)
    set(PATHFINDER_VULKAN_SUPPORT ON)
endif ()

# No Vulkan support for web.
if (EMSCRIPTEN)
    message("Pathfinder will build for web")

    set(PATHFINDER_VULKAN_SUPPORT OFF)
endif ()

if (PATHFINDER_VULKAN_SUPPORT)
    message("Pathfinder will use Vulkan")

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
    message("Pathfinder will use OpenGL")
endif ()

# Include external and third_party headers.
include_directories(external third_party)

if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
    include(FetchContent)
    FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/floppyhammer/glfw.git
            GIT_TAG master
    )
    FetchContent_GetProperties(glfw)

    if (NOT glfw_POPULATED)
        message("Fetching external git repositories")
        FetchContent_Populate(glfw)
    endif ()

    # Include GLFW headers.
    # Emscripten has its own version of GLFW.
    include_directories(${glfw_SOURCE_DIR}/include)
    add_subdirectory(${glfw_SOURCE_DIR})
endif ()

# Core library.
add_subdirectory(src)

if (PATHFINDER_BUILD_DEMO)
    message("Pathfinder will build demo")

    # Demo library.
    add_subdirectory(demo/common)

    if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
        add_subdirectory(demo/native)
    elseif (EMSCRIPTEN)
        # Set output directory to the web demo, so we don't have to copy output files manually.
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo/web")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1")

        # For debugging. Enable this to obtain detailed call stack.
        #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gsource-map -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1 --source-map-base http://127.0.0.1:8000/")

        add_subdirectory(demo/web)
    endif ()
endif ()
