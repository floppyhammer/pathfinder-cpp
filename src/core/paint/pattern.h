#ifndef PATHFINDER_PATTERN_H
#define PATHFINDER_PATTERN_H

#include <cstdint>
#include <utility>
#include <vector>

#include "../../common/color.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"
#include "../../gpu/framebuffer.h"
#include "../../gpu/render_pass.h"
#include "effects.h"

//! Raster image patterns.

namespace Pathfinder {

/// A raster image target that can be rendered to and later reused as a pattern.
///
/// This can be useful for creating "stamps" or "symbols" that are rendered once and reused. It can
/// also be useful for image effects that require many paths to be processed at once; e.g. opacity
/// applied to a group of paths.
struct RenderTarget {
    /// Render pass.
    std::shared_ptr<RenderPass> render_pass;
    /// Framebuffer.
    std::shared_ptr<Framebuffer> framebuffer;
    /// The ID of the render target, including the ID of the scene it came from.
    uint32_t id = 0;
    /// The device pixel size of the render target.
    Vec2<uint32_t> size;
};

/// A raster image, in 32-bit RGBA (8 bits per channel), non-premultiplied form.
struct Image {
    Vec2<uint32_t> size;
    std::vector<ColorU> pixels;
    bool is_opaque = true;

    // TODO: This should not be here.
    mutable std::shared_ptr<Texture> texture;
};

/// Where a raster image pattern comes from.
struct PatternSource {
    enum class Type {
        Image,        // From a CPU image.
        RenderTarget, // From a GPU framebuffer.
    } type = Type::RenderTarget;

    /// A image whose pixels are stored in CPU memory.
    Image image;

    /// Previously-rendered vector content.
    ///
    /// This value allows you to render content and then later use that content as a pattern.
    RenderTarget render_target;

    /// Returns true if this pattern is obviously opaque.
    inline bool is_opaque() const {
        if (type == Type::Image) {
            return image.is_opaque;
        } else {
            return true;
        }
    }
};

/// Various flags that determine behavior of a pattern.
struct PatternFlags {
    uint8_t value = 0x0;

    /// If set, the pattern repeats in the X direction. If unset, the base color is used.
    static const uint8_t REPEAT_X = 0x01;

    /// If set, the pattern repeats in the Y direction. If unset, the base color is used.
    static const uint8_t REPEAT_Y = 0x02;

    /// If set, nearest-neighbor interpolation is used when compositing this pattern (i.e. the
    /// image will be pixelated). If unset, bi-linear interpolation is used when compositing
    /// this pattern (i.e. the image will be smooth).
    static const uint8_t NO_SMOOTHING = 0x04;
};

/// A raster image pattern.
struct Pattern {
    PatternSource source;
    Transform2 transform;
    /// (Optional) A pattern doesn't necessarily need a filter.
    std::shared_ptr<PatternFilter> filter;
    PatternFlags flags;

    static inline Pattern from_source(const PatternSource &p_source) {
        return {p_source, Transform2()};
    }

    /// Creates a new pattern from the given image.
    ///
    /// The transform is initialized to the identity transform. There is no filter.
    static inline Pattern from_image(const Image &image) {
        PatternSource source;
        source.type = PatternSource::Type::Image;
        source.image = image;

        return Pattern::from_source(source);
    }

    /// Creates a new pattern from the given render target with the given size.
    ///
    /// The transform is initialized to the identity transform. There is no filter.
    static inline Pattern from_render_target(const RenderTarget &render_target) {
        PatternSource source;
        source.type = PatternSource::Type::RenderTarget;
        source.render_target = render_target;

        return Pattern::from_source(source);
    }

    /// Returns the affine transform applied to this pattern.
    inline Transform2 get_transform() const {
        return transform;
    }

    /// Applies the given transform to this pattern.
    ///
    /// The transform is applied after any existing transform.
    inline void apply_transform(const Transform2 &p_transform) {
        transform = p_transform * transform;
    }

    /// Returns the underlying pixel size of this pattern, not taking transforms into account.
    inline Vec2<uint32_t> get_size() const {
        switch (source.type) {
            case PatternSource::Type::Image:
                return source.image.size;
            case PatternSource::Type::RenderTarget:
                return source.render_target.size;
        }
    }

    /// Applies a filter to this pattern, replacing any previous filter if any.
    inline void set_filter(const PatternFilter &p_filter) {
        filter = std::make_shared<PatternFilter>(p_filter);
    }

    /// Returns true if this pattern repeats in the X direction or false if the base color will be
    /// used when sampling beyond the coordinates of the image.
    bool repeat_x() const;

    /// Set to true if the pattern should repeat in the X direction or false if the base color
    /// should be used when sampling beyond the coordinates of the image.
    void set_repeat_x(bool repeat_x);

    /// Returns true if this pattern repeats in the Y direction or false if the base color will be
    /// used when sampling beyond the coordinates of the image.
    bool repeat_y() const;

    /// Set to true if the pattern should repeat in the Y direction or false if the base color
    /// should be used when sampling beyond the coordinates of the image.
    void set_repeat_y(bool repeat_y);

    /// Returns true if this pattern should use bi-linear interpolation (i.e. the image will be
    /// smooth) when scaled or false if this pattern should use nearest-neighbor interpolation
    /// (i.e. the image will be pixelated).
    bool smoothing_enabled() const;

    /// Set to true if the pattern should use bi-linear interpolation (i.e. should be smooth) when
    /// scaled or false if this pattern should use nearest-neighbor interpolation (i.e. should be
    /// pixelated).
    void set_smoothing_enabled(bool enable);

    /// Returns true if this pattern is obviously fully opaque.
    inline bool is_opaque() const {
        return source.is_opaque();
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_PATTERN_H
