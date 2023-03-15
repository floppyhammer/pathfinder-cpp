#version 310 es

// pathfinder/shaders/fill.vs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#ifdef VULKAN
    layout(binding = 0) uniform bConstantSizes {
#else
    layout(std140) uniform bConstantSizes {
#endif
    vec2 uFramebufferSize; // Fixed as (4096, 1024).
    vec2 uTileSize; // Fixed as (16, 16).
    vec2 uTextureMetadataSize; // Fixed as (1280, 512). Not used here.
    vec2 pad;
};

layout(location=0) in uvec2 aTessCoord; // Vertex coordinates in a quad, fixed.
layout(location=1) in uvec4 aLineSegment; // Line segment from the built batch.
layout(location=2) in uint aTileIndex; // Alpha tile index.

#ifdef VULKAN
layout(location=0) out vec2 vFrom;
layout(location=1) out vec2 vTo;
#else
out vec2 vFrom;
out vec2 vTo;
#endif

/// Tile index -> index coordinates -> pixel coordinates.
vec2 computeTileOffset(uint tileIndex, float stencilTextureWidth, vec2 tileSize) {
    // Tiles count per row in the mask texture.
    uint tilesPerRow = uint(stencilTextureWidth / tileSize.x);

    // Tile index coordinates.
    uvec2 tileOffset = uvec2(tileIndex % tilesPerRow, tileIndex / tilesPerRow);

    // Pixel coordinates of the tile's origin.
    // We compress data into RGBA channels in the vertical direction.
    // That's why we have vec2(1.0f, 0.25f) here.
    return vec2(tileOffset) * tileSize * vec2(1.0f, 0.25f);
}

vec4 computeVertexPosition(uint tileIndex,
                           uvec2 tessCoord,
                           uvec4 packedLineSegment,
                           vec2 tileSize,
                           vec2 framebufferSize,
                           out vec2 outFrom,
                           out vec2 outTo) {
    // Pixel coordinates of the tile's origin in the mask texture.
    vec2 tileOrigin = computeTileOffset(tileIndex, framebufferSize.x, tileSize);

    // Unpack the packed integer line segment to a unpacked float one by dividing it by 256.0.
    // 256.0f is used to convert integers back to floats.
    vec4 lineSegment = vec4(packedLineSegment) / 256.0f;

    // This is in the local coordinates of the tile.
    vec2 from = lineSegment.xy, to = lineSegment.zw;

    // This is also in the local coordinates of the tile.
    vec2 position;

    // CORE STEP
    // Default square quad -> the fill quad encircled by the line segment, the bottom bound of the tile,
    // and two vertical auxiliary segments.
    if (tessCoord.x == 0u)
        position.x = floor(min(from.x, to.x)); // Left
    else
        position.x = ceil(max(from.x, to.x)); // Right

    if (tessCoord.y == 0u)
        position.y = floor(min(from.y, to.y)); // Top
    else
        position.y = tileSize.y; // Bottom, which is always the bottom bound of the tile.

    // Compress the fill quad in the vertical direction.
    position.y = floor(position.y * 0.25);

    // Since each fragment corresponds to 4 pixels on a scanline, the varying interpolation will
    // land the fragment halfway between the four-pixel strip, at pixel offset 2.0. But we want to
    // do our coverage calculation on the center of the first pixel in the strip instead, at pixel
    // offset 0.5. This adjustment of 1.5 accomplishes that.
    vec2 offset = vec2(0.0, 1.5) - position * vec2(1.0, 4.0);
    outFrom = from + offset;
    outTo = to + offset;

    // Global pixel position -> normalized UV position.
    vec2 globalPosition = (tileOrigin + position) / framebufferSize * 2.0 - 1.0;

    return vec4(globalPosition, 0.0, 1.0);
}

void main() {
    gl_Position = computeVertexPosition(
        aTileIndex,
        aTessCoord,
        aLineSegment,
        uTileSize,
        uFramebufferSize,
        vFrom,
        vTo
    );
}
