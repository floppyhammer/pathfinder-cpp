cmake_minimum_required(VERSION 3.16)

project(pathfinder
        VERSION 0.1
        LANGUAGES C CXX)

# Set binary output directory.
# (Using CMAKE_CURRENT_SOURCE_DIR is necessary.)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Options.
option(PATHFINDER_BACKEND_OPENGL "Enable OpenGL backend" ON)
option(PATHFINDER_BACKEND_VULKAN "Enable Vulkan backend" ON)
option(PATHFINDER_BUILD_DEMO "Build demo" OFF)

# Identify Linux.
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX ON)
endif ()

# Should handle EMSCRIPTEN before other platforms.
if (EMSCRIPTEN)
    message("[Pathfinder] Build for Web")

    set(PATHFINDER_BACKEND_OPENGL ON)
    # No Vulkan support for web.
    set(PATHFINDER_BACKEND_VULKAN OFF)
elseif (APPLE)
    # No GL support for Mac.
    set(PATHFINDER_BACKEND_OPENGL OFF)
    set(PATHFINDER_BACKEND_VULKAN ON)
elseif (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
endif ()

add_subdirectory(pathfinder)

target_compile_features(pathfinder PUBLIC cxx_std_14)

if (PATHFINDER_BACKEND_VULKAN)
    message("[Pathfinder] Enabled Vulkan backend")

    find_package(Vulkan REQUIRED)

    target_link_libraries(pathfinder PUBLIC Vulkan::Vulkan)

    # This is to please GitHub Actions.
    target_include_directories(pathfinder PUBLIC "${Vulkan_INCLUDE_DIRS}")

    # Don't use add_compile_definitions, which doesn't propagate to parent projects.
    target_compile_definitions(pathfinder PRIVATE PATHFINDER_USE_VULKAN)
else ()
    message("[Pathfinder] Disabled Vulkan backend")
endif ()

if (PATHFINDER_BACKEND_OPENGL)
    message("[Pathfinder] Enabled OpenGL backend")

    # Don't use add_compile_definitions, which doesn't propagate to parent projects.
    target_compile_definitions(pathfinder PRIVATE PATHFINDER_USE_OPENGL)
else ()
    message("[Pathfinder] Disabled OpenGL backend")
endif ()

# Include third_party headers.
target_include_directories(pathfinder PUBLIC "third_party" "third_party/glad/include")

# We don't need GLFW on Android, and Emscripten has its own version of GLFW.
if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
    add_subdirectory("third_party/glfw")
    target_include_directories(pathfinder PUBLIC "third_party/glfw/include")
    target_link_libraries(pathfinder PUBLIC glfw)
endif ()

if (PATHFINDER_BUILD_DEMO)
    message("[Pathfinder] Build demo")

    # Demo common library.
    add_subdirectory(demo/common)

    if ((WIN32 OR LINUX OR APPLE) AND NOT (EMSCRIPTEN OR ANDROID))
        add_subdirectory(demo/native)
    elseif (EMSCRIPTEN)
        # Set output directory to the web demo, so we don't have to copy output files manually.
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/demo/web")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1")

        # For debugging. Enable this to obtain detailed call stack.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gsource-map -s ASSERTIONS=2 -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1 -s NO_DISABLE_EXCEPTION_CATCHING --source-map-base http://127.0.0.1:8000/")

        add_subdirectory(demo/web)
    endif ()
endif ()
