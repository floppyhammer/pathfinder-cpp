#ifndef PATHFINDER_PAINT_H
#define PATHFINDER_PAINT_H

#include <map>
#include <memory>
#include <set>
#include <utility>

#include "../../common/math/rect.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"
#include "../data/data.h"
#include "gradient.h"
#include "pattern.h"

namespace Pathfinder {

/// How an overlay is to be composited over a base color.
enum class PaintCompositeOp {
    /// The source that overlaps the destination, replaces the destination.
    SrcIn,
    /// Destination which overlaps the source, replaces the source.
    DestIn,
};

struct TileBatchTextureInfo {
    uint32_t page_id;
    TextureSamplingFlags sampling_flags;
    PaintCompositeOp composite_op;

    inline bool operator==(const TileBatchTextureInfo &rhs) const {
        return page_id == rhs.page_id && sampling_flags == rhs.sampling_flags && composite_op == rhs.composite_op;
    }

    inline bool operator!=(const TileBatchTextureInfo &rhs) const {
        return !(*this == rhs);
    }
};

RectF rect_to_uv(const RectI &rect, const Vec2F &texture_scale);

/// The contents of an overlay: either a gradient or a pattern.
struct PaintContents {
    enum class Type {
        Gradient,
        Pattern,
    } type = Type::Gradient;

    Gradient gradient;
    Pattern pattern;

    // For being used as key in ordered maps.
    inline bool operator<(const PaintContents &rhs) const {
        bool res = type < rhs.type;

        if (type == rhs.type) {
            if (type == PaintContents::Type::Gradient) {
                // FIXME: incorrect.
                res = false;
            } else {
                res = pattern.source < rhs.pattern.source;
            }
        }

        return res;
    }
};

/// What is to be overlaid on top of a base color.
///
/// An overlay is a gradient or a pattern, plus a composite operation which determines how the
/// gradient or pattern is to be combined with the base color.
struct PaintOverlay {
    PaintCompositeOp composite_op = PaintCompositeOp::SrcIn;
    PaintContents contents;
};

/// Defines how a shape is to be filled: with a solid color, gradient, or pattern.
/// A paint content change usually means a color texture change, which results in multiple tile batches.
struct Paint {
private:
    ColorU base_color;

    // Optional
    std::shared_ptr<PaintOverlay> overlay;

public:
    /// Creates a simple paint from a single base color.
    inline static Paint from_color(const ColorU &color) {
        Paint paint;
        paint.base_color = color;
        return paint;
    }

    /// Creates a paint from a gradient.
    inline static Paint from_gradient(const Gradient &gradient) {
        PaintContents contents;
        contents.type = PaintContents::Type::Gradient;
        contents.gradient = gradient;

        Paint paint;
        paint.base_color = ColorU::white();

        paint.overlay = std::make_shared<PaintOverlay>();
        paint.overlay->composite_op = PaintCompositeOp::SrcIn;
        paint.overlay->contents = contents;

        return paint;
    }

    /// Creates a paint from a raster pattern.
    inline static Paint from_pattern(const Pattern &pattern) {
        PaintContents contents;
        contents.type = PaintContents::Type::Pattern;
        contents.pattern = pattern;

        Paint paint;
        paint.base_color = ColorU::white();

        paint.overlay = std::make_shared<PaintOverlay>();
        paint.overlay->composite_op = PaintCompositeOp::SrcIn;
        paint.overlay->contents = contents;

        return paint;
    }

    /// Returns true if this paint is obviously opaque, via a quick check.
    bool is_opaque() const;

    /// Returns the base color of this paint.
    ColorU get_base_color() const;

    /// Changes the base color of this paint.
    void set_base_color(const ColorU &color);

    /// Returns the paint overlay, which is the portion of the paint on top of the base color.
    std::shared_ptr<PaintOverlay> get_overlay() const;

    /// In order to use Paint as Map keys.
    /// See https://stackoverflow.com/questions/1102392/how-can-i-use-stdmaps-with-user-defined-types-as-key.
    inline bool operator<(const Paint &rhs) const {
        if (overlay && rhs.overlay) {
            return overlay->contents < rhs.overlay->contents;
        } else if (overlay && !rhs.overlay) {
            return false;
        } else if (!overlay && rhs.overlay) {
            return true;
        } else {
            return base_color < rhs.base_color;
        }
    }
};

struct ImageTexelInfo {
    TextureLocation location;
    std::shared_ptr<std::vector<ColorU>> texels;
};

struct PaintMetadata;

struct PaintLocationsInfo {
    std::vector<PaintMetadata> paint_metadata;
    GradientTileBuilder gradient_tile_builder;
    std::vector<ImageTexelInfo> image_texel_info;
    std::set<uint64_t> used_image_hashes;
};

/// Metadata related to the color texture.
struct PaintColorTextureMetadata {
    /// The location of the paint.
    TextureLocation location;

    /// The scale for the page this paint is on.
    Vec2F page_scale;

    /// The transform to apply to screen coordinates to translate them into UVs.
    Transform2 transform;

    /// The sampling mode for the texture.
    TextureSamplingFlags sampling_flags;

    /// The filter to be applied to this paint.
    PaintFilter filter;

    /// How the color texture is to be composited over the base color.
    PaintCompositeOp composite_op;

    /// How much of a border there needs to be around the image.
    ///
    /// The border ensures clamp-to-edge yields the right result.
    Vec2I border;
};

/// Built from paints.
struct PaintMetadata {
    /// (Optional) Metadata associated with the color texture, if applicable.
    std::shared_ptr<PaintColorTextureMetadata> color_texture_metadata;

    /// The base color that the color texture gets mixed into.
    ColorU base_color;

    BlendMode blend_mode;

    /// True if this paint is fully opaque.
    bool is_opaque = true;

    PaintFilter filter() const;

    std::shared_ptr<TileBatchTextureInfo> tile_batch_texture_info() const;
};

} // namespace Pathfinder

#endif // PATHFINDER_PAINT_H
