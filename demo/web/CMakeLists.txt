# Include Pathfinder headers.
include_directories(../../include)

# Build executable.
add_executable(pathfinder-demo-web ${SOURCE_FILES}
        main.cpp
        ../common/app.h
        ../common/app.cpp
        ../common/texture_rect.h
        ../common/texture_rect.cpp)

# Link libraries.
target_link_libraries(pathfinder-demo-web glfw pathfinder)
