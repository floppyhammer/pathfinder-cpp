#version 310 es

#ifdef GL_ES
precision highp float;
precision highp sampler2D;
#endif

#ifdef VULKAN
layout(location = 0) out vec4 oFragColor;

layout(location = 0) in vec2 vUV;

layout(binding = 1) uniform sampler2D uTexture;
#else
out vec4 oFragColor;

in vec2 vUV;

uniform sampler2D uTexture;
#endif

void main() {
    // When drawing to the screen, we need to flip the texture for GL.
#ifdef VULKAN
    oFragColor = texture(uTexture, vUV);
#else
    oFragColor = texture(uTexture, vec2(vUV.x, 1.0f - vUV.y));
#endif
}
