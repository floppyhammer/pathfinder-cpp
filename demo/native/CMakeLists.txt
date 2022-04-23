# Build executable.
add_executable(pathfinder_native_demo ${SOURCE_FILES}
        main.cpp
        ../common/app.h
        ../common/app.cpp
        ../../external/glad/glad.c)

# Link libraries.
if (WIN32) # Windows
    target_link_libraries(pathfinder_native_demo glfw libpathfinder)
    if (VULKAN_SUPPORT)
        target_link_libraries(pathfinder_native_demo Vulkan::Vulkan)
    endif ()
else () # Linux
    # sudo apt install libglfw3-dev
    # -lglfw for glfw
    # -ldl for glad
    # -lGL for opengl
    target_link_libraries(pathfinder_native_demo -lglfw -ldl -lGL libpathfinder)
    if (VULKAN_SUPPORT)
        target_link_libraries(pathfinder_native_demo -lvulkan)
    endif ()
endif ()
