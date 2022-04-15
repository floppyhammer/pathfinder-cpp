# Build executable.
add_executable(pathfinder_native_demo ${SOURCE_FILES}
        main.cpp
        ../common/app.h
        ../common/app.cpp
        ../../external/glad/glad.c)

# Link libraries.
if (WIN32) # Windows
    target_link_libraries(pathfinder_native_demo glfw Vulkan::Vulkan libpathfinder)
else () # Linux
    # sudo apt install libglfw3-dev
    # -lglfw for glfw
    # -ldl for glad
    # -lGL for opengl
    target_link_libraries(pathfinder_native_demo -lglfw -ldl -lGL -lvulkan libpathfinder)
endif ()
