#version 310 es

// pathfinder/shaders/tile.vs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#ifdef GL_ES
precision highp sampler2D;
#endif

#ifdef VULKAN
layout(binding = 0) uniform sampler2D uTextureMetadata; // RGBA16F
layout(binding = 1) uniform sampler2D uZBuffer;
#else
uniform sampler2D uTextureMetadata; // RGBA16F
uniform sampler2D uZBuffer;
#endif

#ifdef VULKAN
layout(binding = 2) uniform bUniform {
#else
layout(std140) uniform bUniform {
#endif
    vec2 uTileSize; // Fixed as (16, 16).
    vec2 uTextureMetadataSize; // Fixed as (1280, 512).
    vec2 uZBufferSize;
    vec2 uMaskTextureSize0; // Not used here.
    vec2 uColorTextureSize0; // Not used here.
    vec2 uFramebufferSize; // Not used here.
    mat4 uTransform;
};

layout(location = 0) in uvec2 aTileOffset; // Tile local coordinates
layout(location = 1) in ivec2 aTileOrigin; // Tile index
layout(location = 2) in uvec4 aMaskTexCoord0;
layout(location = 3) in ivec2 aCtrlBackdrop;
layout(location = 4) in int aPathIndex;
layout(location = 5) in uint aMetadataIndex;

#ifdef VULKAN
layout(location = 0) out vec3 vMaskTexCoord0;
layout(location = 1) out vec2 vColorTexCoord0;
layout(location = 2) out vec4 vBaseColor;
layout(location = 3) out float vTileCtrl;
layout(location = 4) out vec4 vFilterParams0;
layout(location = 5) out vec4 vFilterParams1;
layout(location = 6) out vec4 vFilterParams2;
layout(location = 7) out vec4 vFilterParams3;
layout(location = 8) out vec4 vFilterParams4;
layout(location = 9) out float vCtrl;
#else
out vec3 vMaskTexCoord0;
out vec2 vColorTexCoord0;
out vec4 vBaseColor;
out float vTileCtrl;
out vec4 vFilterParams0;
out vec4 vFilterParams1;
out vec4 vFilterParams2;
out vec4 vFilterParams3;
out vec4 vFilterParams4;
out float vCtrl;
#endif

/// Fetch data from the metadata texture.
vec4 fetchUnscaled(sampler2D srcTexture, vec2 scale, vec2 originCoord, int entry) {
    // Integer metadataEntryCoord needs to be scaled, because texture() only accepts float UV coordinates.
    return texture(srcTexture, (originCoord + vec2(0.5) + vec2(entry, 0)) * scale);
}

/// Fetch rendering info from the metadata texture.
void computeTileVaryings(vec2 position, uint metadataIndex, sampler2D textureMetadata,
    vec2 textureMetadataSize, out vec2 outColorTexCoord0, out vec4 outBaseColor,
    out vec4 outFilterParams0, out vec4 outFilterParams1, out vec4 outFilterParams2,
    out vec4 outFilterParams3, out vec4 outFilterParams4, out int outCtrl) {
    // Prepare UV for the metadata texture. Metadata block size is (10, 1).
    // metadataIndex is the block index. Block map size is (128, 512).
    vec2 metadataScale = vec2(1.0) / textureMetadataSize;

    // Pixel coordinates.
    vec2 metadataEntryCoord = vec2(metadataIndex % 128u * 10u, metadataIndex / 128u);

    // Fetch data via texture().
    vec4 colorTexMatrix0 = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 0);
    vec4 colorTexOffsets = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 1);
    vec4 baseColor       = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 2); // Solid color.
    vec4 filterParams0   = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 3);
    vec4 filterParams1   = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 4);
    vec4 filterParams2   = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 5);
    vec4 filterParams3   = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 6);
    vec4 filterParams4   = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 7);
    vec4 extra           = fetchUnscaled(textureMetadata, metadataScale, metadataEntryCoord, 8); // Blend and composite options.

    // Set color texture coordinates.
    // TODO(floppyhammer): I don't understand this step.
    outColorTexCoord0 = mat2(colorTexMatrix0) * position + colorTexOffsets.xy;

    // Set base color.
    outBaseColor = baseColor;

    // Set filter parameters.
    outFilterParams0 = filterParams0;
    outFilterParams1 = filterParams1;
    outFilterParams2 = filterParams2;
    outFilterParams3 = filterParams3;
    outFilterParams4 = filterParams4;

    // Set blend and composite options.
    outCtrl = int(extra.x);
}

void main() {
    // Global tile coordinates.
    vec2 tileOrigin = vec2(aTileOrigin);

    // Local vertex offset, i.e. (0,0), (0,1), (1,1), (1,0).
    vec2 tileOffset = vec2(aTileOffset);

    // Global vertex position.
    vec2 position = (tileOrigin + tileOffset) * uTileSize;

    // Tile culling.
    // --------------------------------------------------
    // Get the UV coordinates of the tile Z value.
    vec2 zUV = ((tileOrigin + vec2(0.5)) / uZBufferSize) * 255.0;

    // Sample Z value from the Z buffer texture.
    ivec4 zValue = ivec4(texture(uZBuffer, zUV));

    // Note that Z value is packed into a RBGA8 pixel.
    // Unpack it. Compare it with the current path index to see
    // if the current tile is under another opaque tile.
    if (aPathIndex < (zValue.x | (zValue.y << 8) | (zValue.z << 16) | (zValue.w << 24))) {
        // Tile culled.
        gl_Position = vec4(0.0);
        return;
    }
    // --------------------------------------------------

    // Global position of the corresponding mask tile.
    uvec2 maskTileCoord = uvec2(aMaskTexCoord0.x, aMaskTexCoord0.y + 256u * aMaskTexCoord0.z);
    vec2 maskTexCoord0 = (vec2(maskTileCoord) + tileOffset) * uTileSize;

    // aMaskTexCoord0.w != 0u means alpha_tile_id is too large (invalid in that case).
    if (aCtrlBackdrop.y == 0 && aMaskTexCoord0.w != 0u) {
        gl_Position = vec4(0.0);
        return;
    }

    // Blend and composite options.
    int ctrl;
    computeTileVaryings(
        position,
        aMetadataIndex,
        uTextureMetadata,
        uTextureMetadataSize,
        vColorTexCoord0,
        vBaseColor,
        vFilterParams0,
        vFilterParams1,
        vFilterParams2,
        vFilterParams3,
        vFilterParams4,
        ctrl
    );

    vTileCtrl = float(aCtrlBackdrop.x);
    vCtrl = float(ctrl);
    vMaskTexCoord0 = vec3(maskTexCoord0, float(aCtrlBackdrop.y));

    // uTransform converts UV coodinates to screen coodinates.
    gl_Position = uTransform * vec4(position, 0.0, 1.0);
}
