# Include Pathfinder headers.
include_directories(../../include)

# Build executable.
add_executable(web-demo ${SOURCE_FILES}
        main.cpp
        ../common/app.h
        ../common/app.cpp
        ../common/texture_rect.h
        ../common/texture_rect.cpp)

# Link libraries.
target_link_libraries(web-demo pathfinder)
