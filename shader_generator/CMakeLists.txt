file(GLOB SRC src/*.cpp ../pathfinder/gpu/shader.cpp ../pathfinder/gpu/shader_translator.cpp)

add_executable(pathfinder_shader_generator ${SRC})

target_compile_definitions(pathfinder_shader_generator PRIVATE PATHFINDER_USE_GLSLANG)

target_include_directories(pathfinder_shader_generator PRIVATE "../third_party/CLI11/include")
target_include_directories(pathfinder_shader_generator PRIVATE "../third_party")

target_link_libraries(pathfinder_shader_generator PRIVATE
        glslang::glslang
        glslang::SPIRV
        glslang::glslang-default-resource-limits
        spirv-cross-c
        spirv-cross-core
        spirv-cross-cpp
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-msl
        spirv-cross-reflect
        spirv-cross-util
)
