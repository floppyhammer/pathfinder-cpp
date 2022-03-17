# Recursive search and automatic re-configuration on adding or deleting source files.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

# Compile as static library. Add prefix "lib" to keep consistence with Android.
add_library(libpathfinder ${SOURCE_FILES})
