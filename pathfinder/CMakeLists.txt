# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE PF_SOURCE_FILES CONFIGURE_DEPENDS common/*.cpp core/*.cpp gpu_mem/*.cpp)

file(GLOB PF_SOURCE_FILES ${PF_SOURCE_FILES} gpu/*.cpp)

if (PATHFINDER_BACKEND_VULKAN)
    file(GLOB PF_SOURCE_FILES ${PF_SOURCE_FILES} gpu/vk/*.cpp)
endif ()

if (PATHFINDER_BACKEND_OPENGL)
    file(GLOB PF_SOURCE_FILES ${PF_SOURCE_FILES} gpu/gl/*.cpp)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        file(GLOB PF_SOURCE_FILES ${PF_SOURCE_FILES} ../third_party/glad/src/gles2.c)
    else ()
        file(GLOB PF_SOURCE_FILES ${PF_SOURCE_FILES} ../third_party/glad/src/gl.c)
    endif ()
endif ()

# Compile as static library.
add_library(pathfinder STATIC ${PF_SOURCE_FILES})
