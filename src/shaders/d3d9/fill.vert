//#version 330
#version 300 es

// pathfinder/shaders/fill.vs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

uniform vec2 uFramebufferSize; // (4096, 1024), fixed size.
uniform vec2 uTileSize; // (16, 16), fixed size.

layout(location=0) in uvec2 aTessCoord; // Vertex coordinates in a quad, fixed.
layout(location=1) in uvec4 aLineSegment; // Line segment from the built batch.
layout(location=2) in uint aTileIndex; // Tile index.

out vec2 vFrom;
out vec2 vTo;

/// Tile index -> tile coordinates -> world coordinates.
vec2 computeTileOffset(uint tileIndex, float stencilTextureWidth, vec2 tileSize) {
    // Tiles count per row in the mask texture.
    uint tilesPerRow = uint(stencilTextureWidth / tileSize.x);

    // Tile coordinates.
    uvec2 tileOffset = uvec2(tileIndex % tilesPerRow, tileIndex / tilesPerRow);

    // vec2(1.0, 0.25) because of stroing data in RGBA channels?
    return vec2(tileOffset) * tileSize * vec2(1.0, 0.25);
}

vec4 computeVertexPosition(uint tileIndex,
                           uvec2 tessCoord,
                           uvec4 packedLineSegment,
                           vec2 tileSize,
                           vec2 framebufferSize,
                           out vec2 outFrom,
                           out vec2 outTo) {
    // World coordinates of the tile in the mask texture.
    vec2 tileOrigin = computeTileOffset(tileIndex, framebufferSize.x, tileSize);

    // Unpack the packed integer line segment to a unpacked float one by dividing it by 256.0.
    vec4 lineSegment = vec4(packedLineSegment) / 256.0f;
    vec2 from = lineSegment.xy, to = lineSegment.zw;

    // Local position to the tile.
    vec2 position;

    // Default quad -> bound rect of the line segment.
    if (tessCoord.x == 0u)
        position.x = floor(min(from.x, to.x));
    else
        position.x = ceil(max(from.x, to.x));

    if (tessCoord.y == 0u)
        position.y = floor(min(from.y, to.y));
    else
        position.y = tileSize.y; // Scanline from above.

    // Compress the rows.
    position.y = floor(position.y * 0.25);

    // Since each fragment corresponds to 4 pixels on a scanline, the varying interpolation will
    // land the fragment halfway between the four-pixel strip, at pixel offset 2.0. But we want to
    // do our coverage calculation on the center of the first pixel in the strip instead, at pixel
    // offset 0.5. This adjustment of 1.5 accomplishes that.
    vec2 offset = vec2(0.0, 1.5) - position * vec2(1.0, 4.0);
    outFrom = from + offset;
    outTo = to + offset;

    // Global screen position -> normalized viewport position.
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
