#version 310 es
//#version 300 es (For GLES)
//#version 330 (For GL)
//#version 310 es (For Vulkan)

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
    oFragColor = texture(uTexture, vUV);
}
