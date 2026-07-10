#version 450

// pathfinder/shaders/tile_clip_copy.fs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

precision highp float;
precision highp sampler2D;

layout(binding = 1) uniform sampler2D uSrc;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 oFragColor;

void main() {
    oFragColor = texture(uSrc, vTexCoord);
}
