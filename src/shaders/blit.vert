R"(
#version 300 es

precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform float uTime;
uniform vec3 uIsoFactor;
uniform mat4 uMvpMat;

out vec3 vColor;
out vec2 vUV;

void main() {
    vUV = aUV;
    vColor = aColor;

    vec3 pixelPos = aPos * uIsoFactor;

    gl_Position = uMvpMat * vec4(pixelPos, 1.0);
}
)"
