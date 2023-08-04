cmake_minimum_required(VERSION 3.4.1)

project(android_demo_vulkan)

# Integrate GameActivity, refer to
#     https://d.android.com/games/agdk/integrate-game-activity
# for the detailed instructions.
find_package(game-activity REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall \
                     -DVK_USE_PLATFORM_ANDROID_KHR")

add_compile_definitions(PATHFINDER_USE_VULKAN)

include_directories(../../../../../../include)
include_directories(../../../../../../third_party)
include_directories(../../../../../../demo/common)

set(PATHFINDER_BUILD_DEMO ON)
add_subdirectory(../../../../../../ pathfinder)

link_directories(lib/${ANDROID_ABI})

add_library(${CMAKE_PROJECT_NAME} SHARED
        VulkanMain.cpp
        AndroidMain.cpp
        vulkan_wrapper.cpp
        GameActivitySources.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        game-activity::game-activity
        log
        android
        pathfinder
        pathfinder_demo_common)
