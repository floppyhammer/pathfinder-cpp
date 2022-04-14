#version 300 es
//#version 330

// pathfinder/shaders/fill.fs.glsl
//
// Copyright © 2020 The Pathfinder Project Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#ifdef GL_ES
precision highp float;
precision highp sampler2D;
#endif

// Pre-prepared texture "area-lut.png" of size (256, 256).
uniform sampler2D uAreaLUT;

in vec2 vFrom;
in vec2 vTo;

out vec4 oFragColor;

/// Understanding this process is quite hard as we need to understand the areaLUT texture first.
/// But I guess areaLUT is mostly used for anti-aliasing.
vec4 computeCoverage(vec2 from, vec2 to, sampler2D areaLUT) {
    // Determine winding, and sort into a consistent order so we only need to find one root below.
    vec2 left = from.x < to.x ? from : to, right = from.x < to.x ? to : from;

    // Shoot a vertical ray toward the curve.
    vec2 window = clamp(vec2(from.x, to.x), -0.5, 0.5);
    float offset = mix(window.x, window.y, 0.5) - left.x;

    // On-segment coordinate.
    float t = offset / (right.x - left.x);

    // Compute position and derivative to form a line approximation.
    float y = mix(left.y, right.y, t); // Scanline hit position y calculated from t.
    float d = (right.y - left.y) / (right.x - left.x); // Derivative of the line segment.

    // Look up area under that line, and scale horizontally to the window size.
    float dX = window.x - window.y;

    // Sample data from the areaLUT texture.
    return texture(areaLUT, vec2(y + 8.0, abs(d * dX)) / 16.0) * dX;
}

void main() {
    oFragColor = computeCoverage(vFrom, vTo, uAreaLUT);
}
