#version 450

layout(location = 0) out vec2 vUV;

void main() {
    float x = -1.0 + float((gl_VertexIndex & 1) << 2);
    float y = -1.0 + float((gl_VertexIndex & 2) << 1);

    gl_Position = vec4(x, y, 0.0, 1.0);

    // Maps clip space coordinates [-1, 1] to UV space [0, 1]
    vUV = gl_Position.xy * 0.5 + 0.5;
}
