#ifndef PATHFINDER_PATTERN_H
#define PATHFINDER_PATTERN_H

//! Raster image patterns.

#include <cstdint>
#include <utility>
#include <vector>

#include "../../common/color.h"
#include "../../common/math/basic.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"
#include "../../gpu/texture.h"
#include "effects.h"

namespace Pathfinder {

/// Identifies a drawing surface for vector graphics that can be later used as a pattern.
struct RenderTargetId {
    /// The ID of the scene that this render target ID belongs to.
    uint32_t scene;
    /// The ID of the render target within this scene.
    uint32_t render_target;

    std::shared_ptr<uint64_t> raw_texture_id;

    bool operator<(const RenderTargetId &rhs) const {
        return scene < rhs.scene && render_target < rhs.render_target;
    }
};

/// A raster image, in 32-bit RGBA (8 bits per channel), non-premultiplied form.
class Image {
public:
    Vec2I size;
    std::vector<ColorU> pixels;
    uint64_t pixels_hash;

    Image(Vec2I _size, const std::vector<ColorU> &_pixels) {
        size = _size;
        pixels = _pixels;
        pixels_hash = fnv_hash(reinterpret_cast<const char *>(pixels.data()), pixels.size() * 4);
    }

    /// Returns a non-cryptographic hash of the image, which should be globally unique.
    uint64_t get_hash() const {
        return pixels_hash;
    }

    // For being used as key in ordered maps.
    bool operator<(const Image &rhs) const {
        bool res = size.x < rhs.size.x;
        res = res && size.y < rhs.size.y;
        res = res && pixels_hash < rhs.pixels_hash;

        return res;
    }
};

/// A raster image target that can be rendered to and later reused as a pattern.
///
/// This can be useful for creating "stamps" or "symbols" that are rendered once and reused. It can
/// also be useful for image effects that require many paths to be processed at once; e.g. opacity
/// applied to a group of paths.
struct RenderTargetDesc {
    Vec2I size;

    std::string name;

    bool is_raw_texture = false;
};

/// Where a raster image pattern comes from.
struct PatternSource {
    enum class Type {
        /// CPU image.
        Image,
        /// GPU framebuffer.
        RenderTarget,
        /// GPU texture.
        Texture,
    } type = Type::Image;

    /// An image whose pixels are stored in CPU memory.
    std::shared_ptr<Image> image;

    /// Previously-rendered vector content.
    ///
    /// This value allows you to render content and then later use that content as a pattern.
    RenderTargetId render_target_id{};
    Vec2I size;

    std::weak_ptr<Texture> texture;

    /// Returns true if this pattern is obviously opaque.
    bool is_opaque() const {
        // We assume all images and render targets are opaque for the sake of simplicity.
        return true;
    }

    // For being used as key in ordered maps.
    bool operator<(const PatternSource &rhs) const {
        if (type == rhs.type) {
            if (type == Type::Image) {
                return *image < *rhs.image;
            } else if (type == Type::RenderTarget) {
                bool res = render_target_id < rhs.render_target_id;
                res = res && size < rhs.size;
                return res;
            } else {
                return texture.lock().get() < rhs.texture.lock().get();
            }
        } else {
            return type < rhs.type;
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

    static Pattern from_source(const PatternSource &source) {
        return {source, Transform2()};
    }

    /// Creates a new pattern from the given image.
    ///
    /// The transform is initialized to the identity transform. There is no filter.
    static Pattern from_image(const std::shared_ptr<Image> &image) {
        PatternSource source;
        source.type = PatternSource::Type::Image;
        source.image = image;

        return Pattern::from_source(source);
    }

    /// Creates a new pattern from the given render target with the given size.
    ///
    /// The transform is initialized to the identity transform. There is no filter.
    static Pattern from_render_target(RenderTargetId id, Vec2I size) {
        PatternSource source;
        source.type = PatternSource::Type::RenderTarget;
        source.render_target_id = id;
        source.size = size;

        return from_source(source);
    }

    static Pattern from_raw_texture(const std::shared_ptr<Texture> &texture) {
        PatternSource source;
        source.type = PatternSource::Type::Texture;
        source.size = texture->get_size();
        source.texture = texture;

        return from_source(source);
    }

    /// Returns the affine transform applied to this pattern.
    Transform2 get_transform() const {
        return transform;
    }

    /// Applies the given transform to this pattern.
    ///
    /// The transform is applied after any existing transform.
    void apply_transform(const Transform2 &_transform) {
        transform = _transform * transform;
    }

    /// Returns the underlying pixel size of this pattern, not taking transforms into account.
    Vec2I get_size() const {
        switch (source.type) {
            case PatternSource::Type::Image:
                return source.image->size;
            case PatternSource::Type::RenderTarget:
                return source.size;
        }
    }

    /// Applies a filter to this pattern, replacing any previous filter if any.
    void set_filter(const PatternFilter &_filter) {
        filter = std::make_shared<PatternFilter>(_filter);
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
    bool is_opaque() const {
        return source.is_opaque();
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_PATTERN_H
