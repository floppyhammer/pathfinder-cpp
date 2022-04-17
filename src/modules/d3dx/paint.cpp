//
// Created by floppyhammer on 9/27/2021.
//

#include "paint.h"

#include "../../rendering/platform.h"

#include <stdexcept>

namespace Pathfinder {
    // Paint member functions.
    // ---------------------------------------------------
    /// Returns true if this paint is obviously opaque, via a quick check.
    bool Paint::is_opaque() const {
        if (!base_color.is_opaque()) {
            return false;
        }

        if (overlay) {
            auto &content = overlay->contents;
            if (content.gradient) {
                //return content.gradient.is_opaque();
                return true;
            } else if (content.pattern) {
                return content.pattern->is_opaque();
            }
        }

        return true;
    }

    /// Returns the *base color* of this paint.
    ColorU Paint::get_base_color() const {
        return base_color;
    }

    /// Changes the *base color* of this paint.
    void Paint::set_base_color(ColorU p_color) {
        base_color = p_color;
    }

    std::shared_ptr<PaintOverlay> Paint::get_overlay() const {
        return overlay;
    }
    // ---------------------------------------------------

    // Palette member functions.
    // ---------------------------------------------------
    Palette::Palette(uint32_t p_scene_id) : scene_id(p_scene_id) {}

    uint32_t Palette::push_paint(const Paint &paint) {
        // Check if already has this paint.
        auto iter = cache.find(paint);
        if (iter != cache.end())
            return iter->second;

        // Push.
        uint32_t paint_id = paints.size();
        cache.insert({ paint, paint_id });
        paints.push_back(paint);

        return paint_id;
    }

    Paint Palette::get_paint(uint32_t paint_id) const {
        if (paint_id < 0 || paint_id >= paints.size()) {
            throw std::runtime_error(std::string("No paint with that ID!"));
        }

        return paints[paint_id];
    }

    RenderTarget Palette::push_render_target(const Vec2<int> &render_target_size) {
        auto device = Platform::get_singleton().device;

        auto render_pass = device->create_render_pass();
        
        // Create a new framebuffer.
        auto framebuffer = device->create_framebuffer(
                render_target_size.x,
                render_target_size.y,
                TextureFormat::RGBA8,
                DataType::UNSIGNED_BYTE,
                render_pass);

        RenderTarget render_target;
        render_target.id = render_targets.size();
        render_target.render_pass = render_pass;
        render_target.framebuffer = framebuffer;
        render_target.size = {(uint32_t) render_target_size.x, (uint32_t) render_target_size.y};

        render_targets.push_back(framebuffer);

        return render_target;
    }

    std::shared_ptr<Framebuffer> Palette::get_render_target(uint32_t render_target_id) const {
        return render_targets[render_target_id];
    }

    std::vector<TextureMetadataEntry> Palette::build_paint_info() {
        std::vector<PaintMetadata> paint_metadata = assign_paint_locations();

        // Calculate texture transforms.
        calculate_texture_transforms(paint_metadata);

        // Create texture metadata.
        auto texture_metadata_entries = create_texture_metadata(paint_metadata);

        return texture_metadata_entries;
    }

    std::vector<TextureMetadataEntry>
    Palette::create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata) {
        std::vector<TextureMetadataEntry> texture_metadata;
        texture_metadata.reserve(p_paint_metadata.size());

        int index = 0;

        for (const auto &paint_metadata: p_paint_metadata) {
            TextureMetadataEntry entry;

            entry.color_0_transform = paint_metadata.color_texture_metadata.transform;

            if (paint_metadata.color_texture_metadata.filter.type != PaintFilter::Type::None) {
                // Changed from SrcIn to DestIn to get pure shadow.
                entry.color_0_combine_mode = ColorCombineMode::DestIn;
            } else {
                entry.color_0_combine_mode = ColorCombineMode::None;
            }

            entry.base_color = paint_metadata.base_color;
            entry.filter = paint_metadata.filter();
            entry.blend_mode = paint_metadata.blend_mode;

            texture_metadata.push_back(entry);

            index++;
        }

        return texture_metadata;
    }

    std::vector<PaintMetadata> Palette::assign_paint_locations() {
        std::vector<PaintMetadata> paint_metadata;
        paint_metadata.reserve(paints.size());

        // Traverse paints.
        for (const auto &paint: paints) {
            PaintColorTextureMetadata color_texture_metadata;

            auto overlay = paint.get_overlay();

            // If not solid color paint.
            if (overlay) {
                if (overlay->contents.gradient) {
                    auto sampling_flags = TextureSamplingFlags();
                    switch (overlay->contents.gradient->wrap) {
                        case GradientWrap::Repeat: {
                            sampling_flags.value |= TextureSamplingFlags::REPEAT_U;
                        } break;
                        case GradientWrap::Clamp: break;
                    }

                    // FIXME: Incomplete.
                } else if (overlay->contents.pattern) {
                    const auto pattern = overlay->contents.pattern;
                    auto border = Vec2<int>(pattern->repeat_x() ? 0 : 1,
                                            pattern->repeat_y() ? 0 : 1);

                    PaintFilter paint_filter;
                    paint_filter.type = PaintFilter::Type::PatternFilter;
                    paint_filter.pattern_filter = pattern->filter;

                    TextureLocation texture_location;
                    texture_location.rect = Rect<int>(0,
                                                      0,
                                                      pattern->source.render_target.size.x,
                                                      pattern->source.render_target.size.y);

                    color_texture_metadata.location = texture_location;
                    color_texture_metadata.filter = paint_filter;
                    color_texture_metadata.transform = Transform2::from_translation(border.to_f32());
                    color_texture_metadata.composite_op = overlay->composite_op;
                }
            }

            paint_metadata.emplace_back(
                    color_texture_metadata,
                    paint.get_base_color(),
                    BlendMode::SrcOver,
                    paint.is_opaque()
            );
        }

        return paint_metadata;
    }

    Rect<float> rect_to_uv(Rect<int> rect, Vec2<float> texture_scale) {
        return rect.to_f32() * texture_scale;
    }

    void Palette::calculate_texture_transforms(std::vector<PaintMetadata> &p_paint_metadata) {
        for (int i = 0; i < paints.size(); i++) {
            auto &paint = paints[i];
            auto &metadata = p_paint_metadata[i];
            auto &color_texture_metadata = metadata.color_texture_metadata;

            auto texture_rect = color_texture_metadata.location.rect;

            auto texture_scale = Vec2<float>(1.f / texture_rect.width(),
                                             1.f / texture_rect.height());

            auto overlay = paint.get_overlay();
            if (overlay) {
                if (overlay->contents.gradient) {

                } else if (overlay->contents.pattern) {
                    if (overlay->contents.pattern->source.type == PatternSource::Type::RenderTarget) {
                        auto pattern = overlay->contents.pattern;
                        auto texture_origin_uv = rect_to_uv(texture_rect, texture_scale).lower_left();

                        auto transform = Transform2::from_translation(texture_origin_uv)
                                         * Transform2::from_scale(texture_scale * Vec2<float>(1.0, -1.0))
                                         * pattern->transform.inverse();

                        color_texture_metadata.transform = transform;
                    }
                }
            }
        }
    }
    // ---------------------------------------------------
}
