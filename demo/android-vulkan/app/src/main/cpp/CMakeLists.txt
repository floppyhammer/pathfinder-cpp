#[[
Copyright 2022 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]
cmake_minimum_required(VERSION 3.4.1)

project(pathfinder_demo)

# Integrate GameActivity, refer to
#     https://d.android.com/games/agdk/integrate-game-activity
# for the detailed instructions.
find_package(game-activity REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall \
                     -DVK_USE_PLATFORM_ANDROID_KHR")

add_compile_definitions(PATHFINDER_USE_VULKAN)

include_directories(third_party)

link_directories(lib/${ANDROID_ABI})

add_library(${CMAKE_PROJECT_NAME} SHARED
        VulkanMain.cpp
        AndroidMain.cpp
        vulkan_wrapper.cpp
        GameActivitySources.cpp
        demo/common/app.cpp
        demo/common/texture_rect.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        game-activity::game-activity
        log
        android
        pathfinder)
