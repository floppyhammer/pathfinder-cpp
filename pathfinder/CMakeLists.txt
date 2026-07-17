# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE PF_SOURCE_FILES CONFIGURE_DEPENDS
        common/*.cpp core/*.cpp gpu_mem/*.cpp utils/*.cpp)

# 1. Base
file(GLOB GPU_CORE_FILES gpu/*.cpp gpu/*.h)
list(APPEND PF_SOURCE_FILES ${GPU_CORE_FILES})

# 2. Backends
if (PATHFINDER_BACKEND_VULKAN)
    file(GLOB VK_FILES gpu/vk/*.cpp gpu/vk/*.h)
    list(APPEND PF_SOURCE_FILES ${VK_FILES})
endif ()

if (PATHFINDER_BACKEND_METAL)
    file(GLOB MTL_FILES gpu/mtl/*.mm gpu/mtl/*.h)
    list(APPEND PF_SOURCE_FILES ${MTL_FILES})
    # Offscreen related.
    list(APPEND PF_SOURCE_FILES gpu/mtl/offscreen_context.mm gpu/mtl/offscreen_context.h)
endif ()

if (PATHFINDER_BACKEND_OPENGL)
    file(GLOB GL_FILES gpu/gl/*.cpp)
    list(APPEND PF_SOURCE_FILES ${GL_FILES})

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM|ARM64|aarch64")
        list(APPEND PF_SOURCE_FILES ../third_party/glad/src/gles2.c)
    else ()
        list(APPEND PF_SOURCE_FILES ../third_party/glad/src/gl.c)
    endif ()
endif ()

# 3. Remove window related sources
if (NOT PATHFINDER_WINDOW)
    list(LENGTH PF_SOURCE_FILES INITIAL_COUNT)
    list(FILTER PF_SOURCE_FILES EXCLUDE REGEX ".*(window|swapchain|swap_chain).*")
    list(LENGTH PF_SOURCE_FILES FINAL_COUNT)

    math(EXPR REMOVED_COUNT "${INITIAL_COUNT} - ${FINAL_COUNT}")

    message(STATUS "[Pathfinder] Files filtered: ${REMOVED_COUNT}")
endif ()

add_library(pathfinder STATIC ${PF_SOURCE_FILES})
