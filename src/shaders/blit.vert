#version 310 es
//#version 300 es (For GLES)
//#version 330 (For GL)
//#version 310 es (For Vulkan)

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aUV;

#ifdef VULKAN
layout(binding = 0) uniform bUniform {
#else
layout(std140) uniform bUniform {
#endif
    mat4 uMvpMat;
};

#ifdef VULKAN
layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vUV;
#else
out vec3 vColor;
out vec2 vUV;
#endif

void main() {
    vUV = aUV;
    vColor = aColor;

    gl_Position = uMvpMat * vec4(aPos, 1.0);

    // When rendering to screen, the Y coordinate is flipped.
    gl_Position.y *= -1.0;
}
