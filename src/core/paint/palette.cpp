#include "palette.h"

#include "../../common/math/basic.h"
#include "../d3d9/tiler.h"
#include "umHalf.h"

namespace Pathfinder {

// 1.0 / sqrt(2 * pi)
const float SQRT_2_PI_INV = 0.3989422804014327;

const int32_t COMBINER_CTRL_FILTER_RADIAL_GRADIENT = 0x1;
const int32_t COMBINER_CTRL_FILTER_TEXT = 0x2;
const int32_t COMBINER_CTRL_FILTER_BLUR = 0x3;
const int32_t COMBINER_CTRL_FILTER_COLOR_MATRIX = 0x4;

const int32_t COMBINER_CTRL_COLOR_FILTER_SHIFT = 4;
const int32_t COMBINER_CTRL_COLOR_COMBINE_SHIFT = 8;
const int32_t COMBINER_CTRL_COMPOSITE_SHIFT = 10;

struct FilterParams {
    F32x4 p0 = F32x4::splat(0);
    F32x4 p1 = F32x4::splat(0);
    F32x4 p2 = F32x4::splat(0);
    F32x4 p3 = F32x4::splat(0);
    F32x4 p4 = F32x4::splat(0);
    int32_t ctrl = 0;
};

FilterParams compute_filter_params(const PaintFilter &filter,
                                   BlendMode blend_mode,
                                   ColorCombineMode color_combine_mode) {
    // Control bit flags.
    int32_t ctrl = 0;

    // Add flags for blend mode and color combine mode.
    ctrl |= blend_mode_to_composite_ctrl(blend_mode) << COMBINER_CTRL_COMPOSITE_SHIFT;
    ctrl |= color_combine_mode_to_composite_ctrl(color_combine_mode) << COMBINER_CTRL_COLOR_COMBINE_SHIFT;

    FilterParams filter_params;
    filter_params.ctrl = ctrl;

    // Add flag for filter.
    switch (filter.type) {
        case PaintFilter::Type::RadialGradient: {
            auto gradient = filter.gradient_filter;

            filter_params.p0 = F32x4(gradient.line.from(), gradient.line.vector());
            filter_params.p1 = F32x4(gradient.radii, gradient.uv_origin);
            filter_params.ctrl = ctrl | (COMBINER_CTRL_FILTER_RADIAL_GRADIENT << COMBINER_CTRL_COLOR_FILTER_SHIFT);
        } break;
        case PaintFilter::Type::PatternFilter: {
            auto pattern = filter.pattern_filter;

            if (pattern.type == PatternFilter::Type::Blur) {
                auto sigma = pattern.blur.sigma;
                auto direction = pattern.blur.direction;

                if (sigma <= 0) {
                    break;
                }

                auto sigma_inv = 1.f / sigma;
                auto gauss_coeff_x = SQRT_2_PI_INV * sigma_inv;
                auto gauss_coeff_y = std::exp(-0.5f * sigma_inv * sigma_inv);
                auto gauss_coeff_z = gauss_coeff_y * gauss_coeff_y;

                auto src_offset = direction == BlurDirection::X ? Vec2F(1.0, 0.0) : Vec2F(0.0, 1.0);

                auto support = std::ceil(1.5f * sigma) * 2.f;

                filter_params.p0 = F32x4(src_offset, Vec2F(support, 0.0));
                filter_params.p1 = F32x4(gauss_coeff_x, gauss_coeff_y, gauss_coeff_z, 0.0);
                filter_params.ctrl = ctrl | (COMBINER_CTRL_FILTER_BLUR << COMBINER_CTRL_COLOR_FILTER_SHIFT);
            } else {
                throw std::runtime_error("Text pattern filter is not supported yet!");
            }
        } break;
        default: // No filter.
            break;
    }

    return filter_params;
}

void upload_texture_metadata(const std::shared_ptr<Texture> &metadata_texture,
                             const std::vector<TextureMetadataEntry> &metadata,
                             const std::shared_ptr<Driver> &driver) {
    auto padded_texel_size =
        alignup_i32((int32_t)metadata.size(), TEXTURE_METADATA_ENTRIES_PER_ROW) * TEXTURE_METADATA_TEXTURE_WIDTH * 4;

    std::vector<half> texels;
    texels.reserve(padded_texel_size);

    for (const auto &entry : metadata) {
        auto base_color = entry.base_color.to_f32();

        auto filter_params = compute_filter_params(entry.filter, entry.blend_mode, entry.color_combine_mode);

        // 40 f16 points, 10 RGBA pixels in total.
        std::array<half, 40> slice = {
            // 0 pixel
            entry.color_transform.m11(),
            entry.color_transform.m21(),
            entry.color_transform.m12(),
            entry.color_transform.m22(),
            // 1 pixel
            entry.color_transform.m13(),
            entry.color_transform.m23(),
            0.0f,
            0.0f,
            // 2 pixel
            base_color.r,
            base_color.g,
            base_color.b,
            base_color.a,
            // 3 pixel
            filter_params.p0.xy().x,
            filter_params.p0.xy().y,
            filter_params.p0.zw().x,
            filter_params.p0.zw().y,
            // 4 pixel
            filter_params.p1.xy().x,
            filter_params.p1.xy().y,
            filter_params.p1.zw().x,
            filter_params.p1.zw().y,
            // 5 pixel
            filter_params.p2.xy().x,
            filter_params.p2.xy().y,
            filter_params.p2.zw().x,
            filter_params.p2.zw().y,
            // 6 pixel
            filter_params.p3.xy().x,
            filter_params.p3.xy().y,
            filter_params.p3.zw().x,
            filter_params.p3.zw().y,
            // 7 pixel
            filter_params.p4.xy().x,
            filter_params.p4.xy().y,
            filter_params.p4.zw().x,
            filter_params.p4.zw().y,
            // 8 pixel
            (float)filter_params.ctrl,
            0.0f,
            0.0f,
            0.0f,
            // 9 pixel
            0.0f,
            0.0f,
            0.0f,
            0.0f,
        };

        texels.insert(texels.end(), slice.begin(), slice.end());
    }

    // Add padding.
    while (texels.size() < padded_texel_size) {
        texels.emplace_back(0.0f);
    }

    // Update the region that contains info instead of the whole texture.
    auto region_rect =
        Rect<uint32_t>(0, 0, TEXTURE_METADATA_TEXTURE_WIDTH, texels.size() / (4 * TEXTURE_METADATA_TEXTURE_WIDTH));

    // Don't use a vector as we need to delay the de-allocation until the image data is uploaded to GPU.
    auto raw_texels = new half[texels.size()];
    std::copy(texels.begin(), texels.end(), raw_texels);

    // Callback to clean up staging resources.
    auto callback = [raw_texels] { delete[] raw_texels; };

    auto cmd_buffer = driver->create_command_buffer(true);
    cmd_buffer->add_callback(callback);
    cmd_buffer->upload_to_texture(metadata_texture, region_rect, raw_texels, TextureLayout::ShaderReadOnly);
    cmd_buffer->submit(driver);
}

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
    auto render_pass =
        driver->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Clear, TextureLayout::ShaderReadOnly);

    // Create a new framebuffer.
    auto target_texture = driver->create_texture(render_target_size.x, render_target_size.y, TextureFormat::Rgba8Unorm);
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

std::vector<PaintMetadata> Palette::build_paint_info(const std::shared_ptr<Driver> &driver) {
    std::vector<PaintMetadata> paint_metadata = assign_paint_locations(driver);

    // Calculate texture transforms.
    calculate_texture_transforms(paint_metadata);

    // Create texture metadata.
    auto texture_metadata_entries = create_texture_metadata(paint_metadata);

    // We only allocate the metadata texture once.
    if (metadata_texture == nullptr) {
        metadata_texture = driver->create_texture(TEXTURE_METADATA_TEXTURE_WIDTH,
                                                  TEXTURE_METADATA_TEXTURE_HEIGHT,
                                                  TextureFormat::Rgba16Float);
    }

    // Upload texture metadata.
    upload_texture_metadata(metadata_texture, texture_metadata_entries, driver);

    return paint_metadata;
}

std::vector<TextureMetadataEntry> Palette::create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata) {
    std::vector<TextureMetadataEntry> texture_metadata;
    texture_metadata.reserve(p_paint_metadata.size());

    for (const auto &paint_metadata : p_paint_metadata) {
        TextureMetadataEntry entry;

        if (paint_metadata.color_texture_metadata) {
            entry.color_transform = paint_metadata.color_texture_metadata->transform;

            // Changed from SrcIn to DestIn to get pure shadow.
            entry.color_combine_mode = ColorCombineMode::SrcIn;
        } else {
            // No color combine mode if there's no need to mix with a color texture.
            entry.color_combine_mode = ColorCombineMode::None;
        }

        entry.base_color = paint_metadata.base_color;
        entry.filter = paint_metadata.filter();
        entry.blend_mode = paint_metadata.blend_mode;

        texture_metadata.push_back(entry);
    }

    return texture_metadata;
}

std::vector<PaintMetadata> Palette::assign_paint_locations(const std::shared_ptr<Driver> &driver) {
    std::vector<PaintMetadata> paint_metadata;
    paint_metadata.reserve(paints.size());

    // For gradient color texture.
    GradientTileBuilder gradient_tile_builder;

    auto gradient_tile_texture =
        driver->create_texture(GRADIENT_TILE_LENGTH, GRADIENT_TILE_LENGTH, TextureFormat::Rgba8Unorm);

    // Traverse paints.
    for (const auto &paint : paints) {
        std::shared_ptr<PaintColorTextureMetadata> color_texture_metadata;

        auto overlay = paint.get_overlay();

        // If not a solid color paint.
        if (overlay) {
            // For the color texture used in the shaders.
            color_texture_metadata = std::make_shared<PaintColorTextureMetadata>();

            if (overlay->contents.type == PaintContents::Type::Gradient) {
                auto &gradient = overlay->contents.gradient;

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

                gradient.tile_texture = gradient_tile_texture;
            } else { // Pattern
                const auto &pattern = overlay->contents.pattern;

                auto border = Vec2<int>(pattern.repeat_x() ? 0 : 1, pattern.repeat_y() ? 0 : 1);

                TextureLocation texture_location;
                {
                    // Image
                    if (pattern.source.type == PatternSource::Type::Image) {
                        texture_location.rect = Rect<uint32_t>({}, pattern.source.image.size);

                        pattern.source.image.texture = driver->create_texture(pattern.source.image.size.x,
                                                                              pattern.source.image.size.y,
                                                                              TextureFormat::Rgba8Unorm);

                        auto cmd_buffer = driver->create_command_buffer(true);

                        cmd_buffer->upload_to_texture(
                            pattern.source.image.texture,
                            Rect<uint32_t>(0, 0, pattern.source.image.size.x, pattern.source.image.size.y),
                            pattern.source.image.pixels.data(),
                            TextureLayout::ShaderReadOnly);

                        cmd_buffer->submit(driver);
                    } else { // Render target
                        texture_location.rect = Rect<uint32_t>({}, pattern.source.render_target.size);
                    }
                }

                PaintFilter paint_filter;
                {
                    // We can have a pattern without a filter.
                    if (pattern.filter == nullptr) {
                        paint_filter.type = PaintFilter::Type::None;
                    } else {
                        paint_filter.type = PaintFilter::Type::PatternFilter;
                        paint_filter.pattern_filter = *pattern.filter;
                    }
                }

                TextureSamplingFlags sampling_flags;
                {
                    if (pattern.repeat_x()) {
                        sampling_flags.value |= TextureSamplingFlags::REPEAT_U;
                    }
                    if (pattern.repeat_y()) {
                        sampling_flags.value |= TextureSamplingFlags::REPEAT_V;
                    }
                    if (!pattern.smoothing_enabled()) {
                        sampling_flags.value |= TextureSamplingFlags::NEAREST_MIN | TextureSamplingFlags::NEAREST_MAG;
                    }
                }

                color_texture_metadata->location = texture_location;
                //                color_texture_metadata->page_scale = ;
                color_texture_metadata->sampling_flags = sampling_flags;
                color_texture_metadata->filter = paint_filter;
                color_texture_metadata->transform = Transform2::from_translation(border.to_f32());
                color_texture_metadata->composite_op = overlay->composite_op;
                color_texture_metadata->border = border;
            }
        }

        paint_metadata.push_back(
            {color_texture_metadata, paint.get_base_color(), BlendMode::SrcOver, paint.is_opaque()});
    }

    gradient_tile_builder.upload(driver, gradient_tile_texture);

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

        auto overlay = paint.get_overlay();

        // Calculate transform.
        if (overlay) {
            if (overlay->contents.type == PaintContents::Type::Gradient) {
                auto gradient_geometry = overlay->contents.gradient.geometry;

                // TODO: Use a texture manager.
                auto texture_scale = Vec2F(1.f / GRADIENT_TILE_LENGTH, 1.f / GRADIENT_TILE_LENGTH);

                color_texture_metadata->page_scale = texture_scale;

                // Convert linear to radical.
                if (gradient_geometry.type == GradientGeometry::Type::Linear) {
                    auto gradient_line = gradient_geometry.linear;
                    auto v0 = texture_rect.to_f32().center().y * texture_scale.y;

                    auto dp = gradient_line.vector();
                    auto m0 = dp / gradient_line.square_length();
                    auto m13 = m0 * -gradient_line.from();

                    color_texture_metadata->transform = Transform2({m0.x, 0.0, m0.y, 0.0}, {m13.x + m13.y, v0});
                } else { // Radical
                    color_texture_metadata->transform = gradient_geometry.radial.transform.inverse();
                }
            } else {
                auto pattern = overlay->contents.pattern;

                Transform2 transform;

                if (pattern.source.type == PatternSource::Type::Image) {
                    auto texture_scale = Vec2F(1.f / texture_rect.width(), 1.f / texture_rect.height());

                    auto texture_origin_uv = rect_to_uv(texture_rect, texture_scale).origin();

                    transform = Transform2::from_scale(texture_scale).translate(texture_origin_uv) *
                                pattern.transform.inverse();
                } else {
                    auto texture_scale = Vec2F(1.f / texture_rect.width(), 1.f / texture_rect.height());

                    auto texture_origin_uv = rect_to_uv(texture_rect, texture_scale).lower_left();

                    transform = Transform2::from_translation(texture_origin_uv) *
                                Transform2::from_scale(texture_scale * Vec2F(1.0, -1.0)) * pattern.transform.inverse();
                };

                color_texture_metadata->transform = transform;
            }
        } else {
            throw std::runtime_error("Why do we have color texture metadata but no overlay?");
        }
    }
}

} // namespace Pathfinder
