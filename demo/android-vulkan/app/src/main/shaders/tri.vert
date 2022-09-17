#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

layout (location = 0) out vec2 vUv;

void main() {
    vUv = aUv;

    gl_Position = vec4(aPos, 0, 1);
}
