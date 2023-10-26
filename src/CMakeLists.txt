# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS common/*.cpp core/*.cpp gpu_mem/*.cpp)

file(GLOB BACKEND_SOURCE_FILES_A CONFIGURE_DEPENDS gpu/*.cpp)

if (PATHFINDER_VULKAN_SUPPORT)
    file(GLOB BACKEND_SOURCE_FILES_B CONFIGURE_DEPENDS gpu/vk/*.cpp)
else ()
    file(GLOB BACKEND_SOURCE_FILES_B CONFIGURE_DEPENDS gpu/gl/*.cpp)
endif ()

# Compile as static library.
add_library(pathfinder ${SOURCE_FILES} ${BACKEND_SOURCE_FILES_A} ${BACKEND_SOURCE_FILES_B})
