#include "palette.h"

namespace Pathfinder {

Palette::Palette(uint32_t p_scene_id) : scene_id(p_scene_id) {}

uint32_t Palette::push_paint(const Paint &paint) {
    // Check if already has this paint.
    // FIXME: Only for pure-color paints for now.
    if (paint.get_overlay() == nullptr) {
        auto iter = cache.find(paint);
        if (iter != cache.end()) {
            return iter->second;
        }
    }

    // Push.
    uint32_t paint_id = paints.size();
    cache.insert({paint, paint_id});
    paints.push_back(paint);

    return paint_id;
}

Paint Palette::get_paint(uint32_t paint_id) const {
    if (paint_id >= paints.size()) {
        throw std::runtime_error(std::string("No paint with that ID!"));
    }

    return paints[paint_id];
}

RenderTarget Palette::push_render_target(const std::shared_ptr<Driver> &driver, const Vec2<int> &render_target_size) {
    auto render_pass = driver->create_render_pass(TextureFormat::RGBA8_UNORM,
                                                  AttachmentLoadOp::CLEAR,
                                                  TextureLayout::SHADER_READ_ONLY);

    // Create a new framebuffer.
    auto target_texture =
        driver->create_texture(render_target_size.x, render_target_size.y, TextureFormat::RGBA8_UNORM);
    auto framebuffer = driver->create_framebuffer(render_pass, target_texture);

    RenderTarget render_target;
    render_target.id = render_targets.size();
    render_target.render_pass = render_pass;
    render_target.framebuffer = framebuffer;
    render_target.size = {(uint32_t)render_target_size.x, (uint32_t)render_target_size.y};

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

std::vector<TextureMetadataEntry> Palette::create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata) {
    std::vector<TextureMetadataEntry> texture_metadata;
    texture_metadata.reserve(p_paint_metadata.size());

    for (const auto &paint_metadata : p_paint_metadata) {
        TextureMetadataEntry entry;

        if (paint_metadata.color_texture_metadata) {
            entry.color_0_transform = paint_metadata.color_texture_metadata->transform;

            // Changed from SrcIn to DestIn to get pure shadow.
            entry.color_0_combine_mode = ColorCombineMode::DestIn;
        } else {
            // No color combine mode if there's no need to mix with a color texture.
            entry.color_0_combine_mode = ColorCombineMode::None;
        }

        entry.base_color = paint_metadata.base_color;
        entry.filter = paint_metadata.filter();
        entry.blend_mode = paint_metadata.blend_mode;

        texture_metadata.push_back(entry);
    }

    return texture_metadata;
}

std::vector<PaintMetadata> Palette::assign_paint_locations() {
    std::vector<PaintMetadata> paint_metadata;
    paint_metadata.reserve(paints.size());

    // For gradient color texture.
    GradientTileBuilder gradient_tile_builder;

    // Traverse paints.
    for (const auto &paint : paints) {
        std::shared_ptr<PaintColorTextureMetadata> color_texture_metadata;

        auto overlay = paint.get_overlay();

        // If not solid color paint.
        if (overlay) {
            // For the color texture used in the shaders.
            color_texture_metadata = std::make_shared<PaintColorTextureMetadata>();

            if (overlay->contents.type == PaintContents::Type::Gradient) {
                const auto gradient = overlay->contents.gradient;

                TextureSamplingFlags sampling_flags;
                if (gradient.wrap == GradientWrap::Repeat) {
                    sampling_flags.value |= TextureSamplingFlags::REPEAT_U;
                }

                if (gradient.geometry.type == GradientGeometry::Type::Linear) {
                    color_texture_metadata->filter.type = PaintFilter::Type::None;
                } else {
                    color_texture_metadata->filter.type = PaintFilter::Type::RadialGradient;
                    color_texture_metadata->filter.gradient_filter.line = gradient.geometry.radial.line;
                    color_texture_metadata->filter.gradient_filter.radii = gradient.geometry.radial.radii;
                }

                // Assign the gradient to a color texture location.
                color_texture_metadata->location = gradient_tile_builder.allocate(gradient);
                color_texture_metadata->sampling_flags = sampling_flags;
                color_texture_metadata->transform = Transform2();
                color_texture_metadata->composite_op = overlay->composite_op;
                color_texture_metadata->border = Vec2<int>();
            } else { // FIXME: Incomplete.
                const auto pattern = overlay->contents.pattern;

                auto border = Vec2<uint32_t>(pattern.repeat_x() ? 0 : 1, pattern.repeat_y() ? 0 : 1);

                PaintFilter paint_filter;
                paint_filter.type = PaintFilter::Type::PatternFilter;
                paint_filter.pattern_filter = pattern.filter;

                TextureLocation texture_location;

                if (pattern.source.type == PatternSource::Type::RenderTarget) {
                    texture_location.rect = Rect<uint32_t>({}, pattern.source.render_target.size);
                } else {
                    texture_location.rect = Rect<uint32_t>() + border * 2;
                }

                color_texture_metadata->location = texture_location;
                color_texture_metadata->filter = paint_filter;
                color_texture_metadata->transform = Transform2::from_translation(border.to_f32());
                color_texture_metadata->composite_op = overlay->composite_op;
            }
        }

        paint_metadata.push_back(
            {color_texture_metadata, paint.get_base_color(), BlendMode::SrcOver, paint.is_opaque()});
    }

//    gradient_tile_builder.upload(gradient_color_texture);

    return paint_metadata;
}

void Palette::calculate_texture_transforms(std::vector<PaintMetadata> &p_paint_metadata) {
    for (int i = 0; i < paints.size(); i++) {
        auto &paint = paints[i];
        auto &metadata = p_paint_metadata[i];
        auto color_texture_metadata = metadata.color_texture_metadata;

        if (!color_texture_metadata) {
            continue;
        }

        auto texture_rect = color_texture_metadata->location.rect;

        auto texture_scale = Vec2<float>(1.f / texture_rect.width(), 1.f / texture_rect.height());

        auto overlay = paint.get_overlay();
        if (overlay) {
            if (overlay->contents.type == PaintContents::Type::Gradient) {
            } else {
                if (overlay->contents.pattern.source.type == PatternSource::Type::RenderTarget) {
                    auto pattern = overlay->contents.pattern;
                    auto texture_origin_uv = rect_to_uv(texture_rect, texture_scale).lower_left();

                    auto transform = Transform2::from_translation(texture_origin_uv) *
                                     Transform2::from_scale(texture_scale * Vec2<float>(1.0, -1.0)) *
                                     pattern.transform.inverse();

                    color_texture_metadata->transform = transform;
                }
            }
        }
    }
}

} // namespace Pathfinder
