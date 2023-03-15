#version 310 es

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

#ifdef VULKAN
layout(binding = 0) uniform bUniform {
#else
layout(std140) uniform bUniform {
#endif
    mat4 uMvpMat;
};

#ifdef VULKAN
layout(location = 0) out vec2 vUV;
#else
out vec2 vUV;
#endif

void main() {
    vUV = aUV;

    gl_Position = uMvpMat * vec4(aPos, 0.0, 1.0);
}
