# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp)

# Compile as static library.
add_library(pathfinder_demo_common STATIC ${SOURCE_FILES})

target_compile_features(pathfinder_demo_common PUBLIC cxx_std_14)

# Include Pathfinder headers.
target_include_directories(pathfinder_demo_common PUBLIC "../../")

# pathfinder has to be linked to pathfinder_demo_common first to ensure macro propagation.
target_link_libraries(pathfinder_demo_common pathfinder)
