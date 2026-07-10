#version 450

// pathfinder/shaders/tile_clip_copy.vs.glsl
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

layout(binding = 0) uniform bUniform {
    vec2 uTileSize; // Fixed as (16, 16).
    vec2 uFramebufferSize; // Mask framebuffer. Dynamic as (4096, 1024 * page_count).
};

layout(location = 0) in uvec2 aTileOffset;
layout(location = 1) in int aTileIndex;

layout(location = 0) out vec2 vTexCoord;

void main() {
    vec2 position = vec2(ivec2(aTileIndex % 256, aTileIndex / 256) + ivec2(aTileOffset));
    position *= vec2(16.0, 4.0) / uFramebufferSize;

    vTexCoord = position;

    if (aTileIndex < 0) {
        position = vec2(0.0);
    }

    gl_Position = vec4(mix(vec2(-1.0), vec2(1.0), position), 0.0, 1.0);
}
