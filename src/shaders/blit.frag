R"(
#version 300 es

precision highp float;

#ifdef GL_ES
precision highp sampler2D;
#endif

out vec4 oFragColor;

in vec2 vUV;
in vec3 vColor;

uniform sampler2D uTexture;

void main() {
    oFragColor = texture(uTexture, vUV);
}
)"
