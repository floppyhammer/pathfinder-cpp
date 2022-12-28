# Include Pathfinder headers.
include_directories(../../include)

# Build executable.
add_executable(native-demo ${SOURCE_FILES}
        main.cpp
        ../common/app.h
        ../common/app.cpp
        ../common/texture_rect.h
        ../common/texture_rect.cpp)

# Link libraries.
if (WIN32) # Windows
    target_link_libraries(native-demo glfw pathfinder)
    if (VULKAN_SUPPORT)
        target_link_libraries(native-demo Vulkan::Vulkan)
    endif ()
else () # Linux
    # sudo apt install libglfw3-dev
    # -lglfw for glfw
    # -ldl for glad
    # -lGL for opengl
    target_link_libraries(native-demo -lglfw -ldl -lGL -lpathfinder)
    if (VULKAN_SUPPORT)
        target_link_libraries(native-demo -lvulkan)
    endif ()
endif ()
