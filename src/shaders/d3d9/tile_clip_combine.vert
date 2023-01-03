#version 310 es
//#version 300 es (For GLES & WebGL)
//#version 330 (For GL)
//#version 310 es (For Vulkan)

// pathfinder/shaders/tile_clip_combine.vs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

precision highp float;

#ifdef GL_ES
precision highp sampler2D;
#endif

#ifdef VULKAN
layout(binding = 0) uniform bConstantSizes {
#else
layout(std140) uniform bConstantSizes {
#endif
    vec2 uFramebufferSize; // Fixed as (4096, 1024).
    vec2 pad0; // Not used here.
    vec2 pad1;
    vec2 pad2;
};

#ifdef VULKAN
layout(location = 0) in uvec2 aTileOffset;
layout(location = 1) in int aDestTileIndex;
layout(location = 2) in int aDestBackdrop;
layout(location = 3) in int aSrcTileIndex;
layout(location = 4) in int aSrcBackdrop;

layout(location = 0) out vec2 vTexCoord0;
layout(location = 1) out float vBackdrop0;
layout(location = 2) out vec2 vTexCoord1;
layout(location = 3) out float vBackdrop1;
#else
in uvec2 aTileOffset;
in int aDestTileIndex;
in int aDestBackdrop;
in int aSrcTileIndex;
in int aSrcBackdrop;

out vec2 vTexCoord0;
out float vBackdrop0;
out vec2 vTexCoord1;
out float vBackdrop1;
#endif

void main() {
    vec2 destPosition = vec2(ivec2(aDestTileIndex % 256, aDestTileIndex / 256) + ivec2(aTileOffset));
    vec2 srcPosition  = vec2(ivec2(aSrcTileIndex  % 256, aSrcTileIndex  / 256) + ivec2(aTileOffset));
    destPosition *= vec2(16.0, 4.0) / uFramebufferSize;
    srcPosition  *= vec2(16.0, 4.0) / uFramebufferSize;

    vTexCoord0 = destPosition;
    vTexCoord1 = srcPosition;

    vBackdrop0 = float(aDestBackdrop);
    vBackdrop1 = float(aSrcBackdrop);

    if (aDestTileIndex < 0) {
        destPosition = vec2(0.0);
    }

    gl_Position = vec4(mix(vec2(-1.0), vec2(1.0), destPosition), 0.0, 1.0);
}
