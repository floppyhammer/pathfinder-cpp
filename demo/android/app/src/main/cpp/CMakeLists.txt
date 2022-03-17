cmake_minimum_required(VERSION 3.7.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -Wall -fopenmp -static-openmp")
if (${ANDROID_PLATFORM_LEVEL} LESS 12)
  message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
  return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
  add_definitions("-DDYNAMIC_ES3")
  set(GL3STUB_SRC gl3stub.c)
  set(OPENGL_LIB GLESv2)
else ()
  set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

include_directories(third_party)

link_directories(lib/${ANDROID_ABI})

add_library(gles3jni SHARED
        ${GL3STUB_SRC}
        gles3jni.cpp
        RendererES2.cpp
        RendererES3.cpp
        demo/common/app.cpp)

target_link_libraries(gles3jni
        ${OPENGL_LIB}
        pathfinder.a
        android
        EGL
        log
        m)
