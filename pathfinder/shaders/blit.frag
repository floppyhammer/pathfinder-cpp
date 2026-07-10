#version 450

precision highp float;
precision highp sampler2D;

layout(location = 0) out vec4 oFragColor;

layout(location = 0) in vec2 vUV;

layout(binding = 0) uniform sampler2D uTexture;

void main() {
    oFragColor = texture(uTexture, vUV);
}
