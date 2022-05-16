# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

# Compile as static library.
add_library(pathfinder ${SOURCE_FILES})
