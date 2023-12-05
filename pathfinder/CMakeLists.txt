# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS common/*.cpp core/*.cpp gpu_mem/*.cpp)

file(GLOB SOURCE_FILES ${SOURCE_FILES} gpu/*.cpp)

if (PATHFINDER_VULKAN_SUPPORT)
    file(GLOB SOURCE_FILES ${SOURCE_FILES} gpu/vk/*.cpp)
else ()
    file(GLOB SOURCE_FILES ${SOURCE_FILES} gpu/gl/*.cpp)
endif ()

# Compile as static library.
add_library(pathfinder ${SOURCE_FILES}
        ../third_party/glad/src/gl.c)
