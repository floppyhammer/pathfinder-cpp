#version 310 es

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

#ifdef VULKAN
layout(location = 0) out vec2 vUV;
#else
out vec2 vUV;
#endif

void main() {
    vUV = aUV;

    gl_Position = vec4(aPos, 0.0, 1.0);
}
