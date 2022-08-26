#ifndef PATHFINDER_PAINT_H
#define PATHFINDER_PAINT_H

#include "gradient.h"
#include "pattern.h"
#include "data/data.h"
#include "data/line_segment.h"
#include "../common/color.h"
#include "../common/math/vec2.h"
#include "../common/math/rect.h"
#include "../common/math/transform2.h"
#include "../gpu/driver.h"

#include <map>
#include <utility>
#include <memory>

namespace Pathfinder {
    /// The size of a gradient tile.
    // TODO(pcwalton): Choose this size dynamically!
    const uint32_t GRADIENT_TILE_LENGTH = 256;

    /// How an overlay is to be composited over a base color.
    enum class PaintCompositeOp {
        /// The source that overlaps the destination, replaces the destination.
        SrcIn,
        /// Destination which overlaps the source, replaces the source.
        DestIn,
    };

    /// The contents of an overlay: either a gradient or a pattern.
    struct PaintContents {
        /// A gradient, either linear or radial.
        std::shared_ptr<Gradient> gradient;

        /// A raster image pattern.
        std::shared_ptr<Pattern> pattern;
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
    struct Paint {
    private:
        ColorU base_color;
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
            contents.gradient = std::make_shared<Gradient>(gradient);

            Paint paint;
            paint.base_color = ColorU::white();

            paint.overlay = std::make_shared<PaintOverlay>();
            paint.overlay->composite_op = PaintCompositeOp::SrcIn;
            paint.overlay->contents = contents;

            return paint;
        }

        /// Creates a paint from a raster pattern.
        inline static Paint from_pattern(const Pattern& pattern) {
            PaintContents contents;
            contents.pattern = std::make_shared<Pattern>(pattern);

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
        void set_base_color(ColorU p_color);

        /// Returns the paint overlay, which is the portion of the paint on top of the base color.
        std::shared_ptr<PaintOverlay> get_overlay() const;

        /// In order to use Paint as Map keys.
        /// See https://stackoverflow.com/questions/1102392/how-can-i-use-stdmaps-with-user-defined-types-as-key.
        inline bool operator<(const Paint &rhs) const {
            int base_color_index_l = base_color.to_u32();
            int base_color_index_r = rhs.base_color.to_u32();

            int overlay_index_l = -1;
            if (overlay && overlay->contents.pattern) {
                overlay_index_l = overlay->contents.pattern->source.render_target.framebuffer->get_unique_id();
            }

            int overlay_index_r = -1;
            if (rhs.overlay && rhs.overlay->contents.pattern) {
                overlay_index_r = rhs.overlay->contents.pattern->source.render_target.framebuffer->get_unique_id();
            }

            if (base_color_index_l < base_color_index_r) {
                return true;
            } else if (base_color_index_l > base_color_index_r) {
                return false;
            } else {
                return overlay_index_l < overlay_index_r;
            }
        }
    };

    struct TextureLocation {
        uint32_t page{};
        Rect<uint32_t> rect;
    };

    struct TextureSamplingFlags {
        uint8_t value = 0;

        static const uint8_t REPEAT_U = 0x01;
        static const uint8_t REPEAT_V = 0x02;
        static const uint8_t NEAREST_MIN = 0x04;
        static const uint8_t NEAREST_MAG = 0x08;
    };

    struct PaintFilter {
        enum class Type {
            None,
            RadialGradient,
            PatternFilter,
        } type = Type::None;

        struct RadialGradient {
            /// The line segment that connects the two circles.
            LineSegmentF line;
            /// The radii of the two circles.
            Vec2<float> radii;
        };

        RadialGradient gradient;

        PatternFilter pattern_filter;
    };

    /// Metadata related to color texture.
    struct PaintColorTextureMetadata {
        /// The location of the paint.
        TextureLocation location;
        /// The scale for the page this paint is on.
        Vec2<float> page_scale;
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
        Vec2<int> border;
    };

    struct PaintMetadata {
        /// Metadata associated with the color texture, if applicable.
        PaintColorTextureMetadata color_texture_metadata;

        /// The base color that the color texture gets mixed into.
        ColorU base_color;

        BlendMode blend_mode;

        /// True if this paint is fully opaque.
        bool is_opaque = true;

        inline Filter filter() const {
            Filter filter;

            switch (color_texture_metadata.filter.type) {
                case PaintFilter::Type::PatternFilter: {
                    filter.type = Filter::Type::PatternFilter;
                    filter.pattern_filter = color_texture_metadata.filter.pattern_filter;
                }
                    break;
                default:
                    break;
            }
            return filter;
        }
    };

    /// Stores all paints in a scene.
    struct Palette {
    public:
        explicit Palette(uint32_t p_scene_id);

        /// Push a new paint if not already in cache,
        /// and return its ID.
        uint32_t push_paint(const Paint &paint);

        Paint get_paint(uint32_t paint_id) const;

        RenderTarget push_render_target(const std::shared_ptr<Driver>& p_driver, const Vec2<int> &render_target_size);

        std::shared_ptr<Framebuffer> get_render_target(uint32_t render_target_id) const;

        /// Core step.
        std::vector<TextureMetadataEntry> build_paint_info();

    private:
        std::vector<Paint> paints;

        std::vector<std::shared_ptr<Framebuffer>> render_targets;

        std::map<Paint, uint32_t> cache;

        uint32_t scene_id;

        std::vector<PaintMetadata> assign_paint_locations();

        void calculate_texture_transforms(std::vector<PaintMetadata> &p_paint_metadata);

        std::vector<TextureMetadataEntry> create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata);
    };
}

#endif //PATHFINDER_PAINT_H
