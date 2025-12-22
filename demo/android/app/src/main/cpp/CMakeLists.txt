cmake_minimum_required(VERSION 3.4.1)

project(pathfinder_android_demo)

# Integrate GameActivity, refer to
#     https://d.android.com/games/agdk/integrate-game-activity
# for the detailed instructions.
find_package(game-activity REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -frtti -Wall \
                     -DVK_USE_PLATFORM_ANDROID_KHR")

include_directories(../../../../../../)
include_directories(../../../../../../third_party)
include_directories(../../../../../../demo/common)

# Set some flags before adding.
set(PATHFINDER_BUILD_DEMO ON CACHE BOOL "" FORCE)
set(PATHFINDER_BACKEND_VULKAN ON CACHE BOOL "" FORCE)
add_subdirectory(../../../../../../ pathfinder)

link_directories(lib/${ANDROID_ABI})

add_library(${CMAKE_PROJECT_NAME} SHARED
        AndroidMain.cpp
        vulkan_wrapper.cpp
        GameActivitySources.cpp
        native_engine.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        game-activity::game-activity
        log
        android
        pathfinder
        pathfinder_demo_common
        EGL # GL
        GLESv3 # GL
)
