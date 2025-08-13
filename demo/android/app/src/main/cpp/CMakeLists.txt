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


################### GL
if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    set(GL3STUB_SRC gl3stub.c)
    set(OPENGL_LIB GLESv2)
else ()
    set(OPENGL_LIB GLESv3)
endif ()
################### GL

link_directories(lib/${ANDROID_ABI})

add_library(${CMAKE_PROJECT_NAME} SHARED
        AndroidMain.cpp
        vulkan_wrapper.cpp
        GameActivitySources.cpp
        native_engine.cpp
        native_engine_gl.cpp
        native_engine_vk.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        game-activity::game-activity
        log
        android
        pathfinder
        pathfinder_demo_common
        EGL # GL
        ${OPENGL_LIB} # GL
)
