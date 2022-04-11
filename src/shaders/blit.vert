R"(
#version 300 es

precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

layout (std140) uniform bUniform {
    mat4 uMvpMat;
};

out vec3 vColor;
out vec2 vUV;

void main() {
    vUV = aUV;
    vColor = aColor;

    gl_Position = uMvpMat * vec4(aPos, 1.0);
}
)"
