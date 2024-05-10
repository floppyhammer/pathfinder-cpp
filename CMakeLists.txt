cmake_minimum_required(VERSION 3.16)

project(pathfinder
        VERSION 0.1
        LANGUAGES C CXX)

# Set binary output directory.
# (Using CMAKE_CURRENT_SOURCE_DIR is necessary.)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Options.
option(PATHFINDER_VULKAN_SUPPORT "Use Vulkan instead of OpenGL" OFF)
option(PATHFINDER_BUILD_DEMO "Build demo" OFF)

# Identify Linux.
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX ON)
endif ()

# Should handle EMSCRIPTEN before other platforms.
if (EMSCRIPTEN)
    message("[Pathfinder] Build for Web")

    # No Vulkan support for web.
    set(PATHFINDER_VULKAN_SUPPORT OFF)
elseif (APPLE)
    # No GL support for Mac.
    set(PATHFINDER_VULKAN_SUPPORT ON)
elseif (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
endif ()

add_subdirectory(pathfinder)

target_compile_features(pathfinder PUBLIC cxx_std_14)

if (PATHFINDER_VULKAN_SUPPORT)
    message("[Pathfinder] Use Vulkan")

    find_package(Vulkan REQUIRED)

    # This is to please GitHub Actions.
    target_include_directories(pathfinder PUBLIC "${Vulkan_INCLUDE_DIRS}")
else ()
    message("[Pathfinder] Use OpenGL")
endif ()

# Include third_party headers.
target_include_directories(pathfinder PUBLIC "third_party" "third_party/glad/include")

# We don't need GLFW on Android, and Emscripten has its own version of GLFW.
if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
    include(FetchContent)
    FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/floppyhammer/glfw.git
            GIT_TAG master
    )
    FetchContent_GetProperties(glfw)

    if (NOT glfw_POPULATED)
        message("[Pathfinder] Fetching external git repositories")
        FetchContent_Populate(glfw)
    endif ()

    target_include_directories(pathfinder PUBLIC "${glfw_SOURCE_DIR}/include")
    target_link_libraries(pathfinder PUBLIC glfw)
endif ()

# Add some macros.
if (PATHFINDER_VULKAN_SUPPORT)
    # Don't use add_compile_definitions, which doesn't propagate to parent projects.
    target_compile_definitions(pathfinder PUBLIC PATHFINDER_USE_VULKAN)
endif ()

# Link libraries.
if (WIN32)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder PUBLIC Vulkan::Vulkan)
    endif ()
elseif (LINUX)
    if (PATHFINDER_VULKAN_SUPPORT)
        target_link_libraries(pathfinder PUBLIC -lvulkan)
    else ()
        target_link_libraries(pathfinder PUBLIC -lGL)
    endif ()
elseif (APPLE)
    target_link_libraries(pathfinder PUBLIC Vulkan::Vulkan)
endif ()

if (PATHFINDER_BUILD_DEMO)
    message("[Pathfinder] Build demo")

    # target_compile_definitions only added PATHFINDER_USE_VULKAN to the Pathfinder library.
    # We need to define PATHFINDER_USE_VULKAN for the demo as well.
    if (PATHFINDER_VULKAN_SUPPORT)
        add_compile_definitions(PATHFINDER_USE_VULKAN)
    endif ()

    # Demo common library.
    add_subdirectory(demo/common)

    if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
        add_subdirectory(${glfw_SOURCE_DIR})
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
