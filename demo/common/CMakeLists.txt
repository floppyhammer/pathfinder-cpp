# Include Pathfinder headers.
include_directories(../../include)

# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

# Compile as static library.
add_library(pathfinder_demo ${SOURCE_FILES})
