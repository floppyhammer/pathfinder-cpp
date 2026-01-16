#include "palette.h"

#include "../../common/math/basic.h"
#include "../d3d9/tiler.h"
#include "../renderer.h"

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

Palette::Palette(uint32_t _scene_id) : scene_id(_scene_id) {}

uint32_t Palette::push_paint(const Paint &paint) {
    // Check if this paint already exists.
    // FIXME: caching is only for pure-color paints for now.
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

RenderTargetId Palette::push_render_target(const RenderTargetDesc &render_target_desc) {
    uint32_t id = render_targets_desc.size();
    render_targets_desc.push_back(render_target_desc);
    return {scene_id, id};
}

RenderTargetDesc Palette::get_render_target(RenderTargetId render_target_id) const {
    return render_targets_desc[render_target_id.render_target];
}

std::vector<PaintMetadata> Palette::build_paint_info(Renderer *renderer) {
    auto paint_texture_manager = std::make_shared<PaintTextureManager>();

    std::vector<TextureLocation> transient_paint_locations;

    // Assign render target locations.
    auto render_target_metadata = assign_render_target_locations(paint_texture_manager, transient_paint_locations);

    PaintLocationsInfo paint_locations_info =
        assign_paint_locations(paint_texture_manager, render_target_metadata, transient_paint_locations);

    // Calculate texture transforms.
    calculate_texture_transforms(paint_locations_info.paint_metadata);

    // Create texture metadata.
    auto texture_metadata_entries = create_texture_metadata(paint_locations_info.paint_metadata);

    auto encoder =
        renderer->device->create_command_encoder("upload palette data (metadata texture & pattern texture pages)");

    // Upload texture metadata.
    renderer->upload_texture_metadata(texture_metadata_entries, encoder);

    // Allocate textures for all images in the paint texture manager.
    allocate_textures(paint_texture_manager, renderer);

    // Render targets.
    for (uint32_t index = 0; index < render_target_metadata.size(); index++) {
        auto &metadata = render_target_metadata[index];
        auto id = RenderTargetId{scene_id, index};
        renderer->declare_render_target(id, metadata);
    }

    // Gradient tiles.
    for (auto &tile : paint_locations_info.gradient_tile_builder.tiles) {
        renderer->upload_texel_data(tile.texels,
                                    TextureLocation{tile.page, RectI(Vec2I(0, 0), Vec2I(GRADIENT_TILE_LENGTH))},
                                    encoder);
    }

    // Image texels.
    std::set<uint32_t> uploaded_image_pages;
    for (auto &texel_info : paint_locations_info.image_texel_info) {
        // Skip repeated image pages.
        if (uploaded_image_pages.find(texel_info.location.page) == uploaded_image_pages.end()) {
            renderer->upload_texel_data(*texel_info.texels, texel_info.location, encoder);
            uploaded_image_pages.insert(texel_info.location.page);
        }
    }

    renderer->queue->submit(encoder, renderer->fence);

    // Free transient locations and unused images, now that they're no longer needed.
    free_transient_locations(*paint_texture_manager, transient_paint_locations);

    // Frees images that are cached but not used this frame.
    free_unused_images(*paint_texture_manager, paint_locations_info.used_image_hashes);

    return paint_locations_info.paint_metadata;
}

std::vector<TextureMetadataEntry> Palette::create_texture_metadata(const std::vector<PaintMetadata> &paint_metadata) {
    std::vector<TextureMetadataEntry> texture_metadata;
    texture_metadata.reserve(paint_metadata.size());

    for (const auto &metadata : paint_metadata) {
        TextureMetadataEntry entry;

        if (metadata.color_texture_metadata) {
            entry.color_transform = metadata.color_texture_metadata->transform;

            // Changed from SrcIn to DestIn to get pure shadow.
            entry.color_combine_mode = ColorCombineMode::SrcIn;
        } else {
            // No color combine mode if there's no need to mix with a color texture.
            entry.color_combine_mode = ColorCombineMode::None;
        }

        entry.base_color = metadata.base_color;
        entry.filter = metadata.filter();
        entry.blend_mode = metadata.blend_mode;

        texture_metadata.push_back(entry);
    }

    return texture_metadata;
}

std::vector<TextureLocation> Palette::assign_render_target_locations(
    const std::shared_ptr<PaintTextureManager> &texture_manager,
    std::vector<TextureLocation> &transient_paint_locations) {
    std::vector<TextureLocation> render_target_metadata;

    for (const auto &desc : render_targets_desc) {
        auto location = texture_manager->allocator.allocate_image(desc.size);
        render_target_metadata.push_back(location);
        transient_paint_locations.push_back(location);
    }

    return render_target_metadata;
}

PaintLocationsInfo Palette::assign_paint_locations(const std::shared_ptr<PaintTextureManager> &texture_manager,
                                                   const std::vector<TextureLocation> &render_target_metadata,
                                                   std::vector<TextureLocation> &transient_paint_locations) {
    std::vector<PaintMetadata> paint_metadata;
    //    paint_metadata.reserve(paints.size());
    GradientTileBuilder gradient_tile_builder;
    std::vector<ImageTexelInfo> image_texel_info;
    std::set<uint64_t> used_image_hashes;

    // Traverse paints.
    for (const auto &paint : paints) {
        auto &allocator = texture_manager->allocator;

        std::shared_ptr<PaintColorTextureMetadata> color_texture_metadata;

        auto overlay = paint.get_overlay();

        // If not a solid color paint.
        if (overlay) {
            color_texture_metadata = std::make_shared<PaintColorTextureMetadata>();

            // Gradient.
            if (overlay->contents.type == PaintContents::Type::Gradient) {
                const auto &gradient = overlay->contents.gradient;

                TextureSamplingFlags sampling_flags;
                if (gradient.wrap == GradientWrap::Repeat) {
                    sampling_flags.value |= TextureSamplingFlags::REPEAT_U;
                }

                // FIXME(pcwalton): The gradient size might not be big enough. Detect this.
                auto location = gradient_tile_builder.allocate(gradient, allocator, transient_paint_locations);

                if (gradient.geometry.type == GradientGeometry::Type::Linear) {
                    color_texture_metadata->filter.type = PaintFilter::Type::None;
                } else {
                    color_texture_metadata->filter.type = PaintFilter::Type::RadialGradient;
                    color_texture_metadata->filter.gradient_filter.line = gradient.geometry.radial.line;
                    color_texture_metadata->filter.gradient_filter.radii = gradient.geometry.radial.radii;
                }

                // Assign the gradient to a color texture location.
                color_texture_metadata->location = location;
                color_texture_metadata->page_scale = allocator.page_scale(location.page);
                color_texture_metadata->sampling_flags = sampling_flags;
                color_texture_metadata->transform = Transform2();
                color_texture_metadata->composite_op = overlay->composite_op;
                color_texture_metadata->border = Vec2I();
            }
            // Pattern.
            else {
                const auto &pattern = overlay->contents.pattern;

                auto border = Vec2I(pattern.repeat_x() ? 0 : 1, pattern.repeat_y() ? 0 : 1);

                TextureLocation location;

                // Render target
                if (pattern.source.type == PatternSource::Type::RenderTarget) {
                    auto render_target_id = pattern.source.render_target_id;

                    auto index = render_target_id.render_target;

                    location = render_target_metadata[index];
                }
                // Image
                else if (pattern.source.type == PatternSource::Type::Image) {
                    auto image = pattern.source.image;

                    // TODO(pcwalton): We should be able to use tile cleverness to
                    // repeat inside the atlas in some cases.
                    auto image_hash = image->get_hash();

                    auto &cached_images = texture_manager->cached_images;

                    // Check cache.
                    if (cached_images.find(image_hash) != cached_images.end()) {
                        location = cached_images[image_hash];
                        // Mark this image cache as being used in this frame.
                        used_image_hashes.insert(image_hash);
                    } else {
                        // Leave a pixel of border on the side.
                        auto allocation_mode = AllocationMode::OwnPage;
                        location = allocator.allocate(image->size + border * 2, allocation_mode);
                        location.rect = location.rect.contract(border);
                        cached_images[image_hash] = location;
                    }

                    image_texel_info.push_back(ImageTexelInfo{
                        TextureLocation{
                            location.page,
                            location.rect,
                        },
                        std::make_shared<std::vector<ColorU>>(image->pixels),
                    });
                }

                TextureSamplingFlags sampling_flags;
                if (pattern.repeat_x()) {
                    sampling_flags.value |= TextureSamplingFlags::REPEAT_U;
                }
                if (pattern.repeat_y()) {
                    sampling_flags.value |= TextureSamplingFlags::REPEAT_V;
                }
                if (!pattern.smoothing_enabled()) {
                    sampling_flags.value |= TextureSamplingFlags::NEAREST_MIN | TextureSamplingFlags::NEAREST_MAG;
                }
                // Raspberry PI only supports NEAREST for NPOT textures.
#if defined(__linux__) && defined(__ARM_ARCH)
                sampling_flags.value |= TextureSamplingFlags::NEAREST_MIN | TextureSamplingFlags::NEAREST_MAG;
#endif

                PaintFilter paint_filter;
                // We can have a pattern without a filter.
                if (pattern.filter == nullptr) {
                    paint_filter.type = PaintFilter::Type::None;
                } else {
                    paint_filter.type = PaintFilter::Type::PatternFilter;
                    paint_filter.pattern_filter = *pattern.filter;
                }

                // Texture
                if (pattern.source.type == PatternSource::Type::Texture) {
                    color_texture_metadata->raw_texture = pattern.source.texture;
                    location.rect = RectI({}, pattern.source.texture.lock()->get_size());
                    // No page info for raw textures.
                } else {
                    color_texture_metadata->page_scale = allocator.page_scale(location.page);
                }

                color_texture_metadata->location = location;
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

    return PaintLocationsInfo{
        paint_metadata,
        gradient_tile_builder,
        image_texel_info,
        used_image_hashes,
    };
}

void Palette::calculate_texture_transforms(std::vector<PaintMetadata> &paint_metadata) {
    for (int i = 0; i < paints.size(); i++) {
        auto &paint = paints[i];
        auto &metadata = paint_metadata[i];
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

                auto texture_scale = Vec2F(1.f / texture_rect.width(), 1.f / texture_rect.height());

                auto texture_origin_uv = rect_to_uv(texture_rect, texture_scale).origin();

                color_texture_metadata->transform =
                    Transform2::from_scale(texture_scale).translate(texture_origin_uv) * pattern.transform.inverse();
            }
        } else {
            throw std::runtime_error("Why do we have color texture metadata but no overlay?");
        }
    }
}

void Palette::free_transient_locations(PaintTextureManager &texture_manager,
                                       const std::vector<TextureLocation> &transient_paint_locations) {
    for (const auto &location : transient_paint_locations) {
        texture_manager.allocator.free(location);
    }
}

// Frees images that are cached but not used this frame.
void Palette::free_unused_images(PaintTextureManager &texture_manager, const std::set<uint64_t> &used_image_hashes) {
    auto &cached_images = texture_manager.cached_images;
    auto &allocator = texture_manager.allocator;

    std::vector<uint64_t> image_hashes_to_remove;
    for (auto &image : cached_images) {
        // Check if this cache has been used in the current frame.
        bool keep = used_image_hashes.find(image.first) != used_image_hashes.end();

        // Free it if it hasn't.
        if (!keep) {
            allocator.free(image.second);
            image_hashes_to_remove.push_back(image.first);
        }
    }

    for (auto &hash : image_hashes_to_remove) {
        cached_images.erase(hash);
    }
}

MergedPaletteInfo Palette::append_palette(const Palette &palette, const Transform2 &transform) {
    // Merge render targets.
    std::map<RenderTargetId, RenderTargetId> render_target_mapping;
    for (uint32_t old_render_target_index = 0; old_render_target_index < palette.render_targets_desc.size();
         old_render_target_index++) {
        auto render_target = palette.render_targets_desc[old_render_target_index];

        RenderTargetId old_render_target_id = {
            palette.scene_id,
            old_render_target_index,
        };

        auto new_render_target_id = push_render_target(render_target);
        render_target_mapping.insert({old_render_target_id, new_render_target_id});
    }

    // Merge paints.
    std::map<uint16_t, uint16_t> paint_mapping;
    for (uint32_t old_paint_index = 0; old_paint_index < palette.paints.size(); old_paint_index++) {
        auto old_paint_id = old_paint_index;
        auto paint = palette.paints[old_paint_index];

        uint32_t new_paint_id;
        if (paint.get_overlay()) {
            auto &contents = paint.get_overlay()->contents;

            if (contents.type == PaintContents::Type::Pattern) {
                if (contents.pattern.source.type == PatternSource::Type::RenderTarget) {
                    auto new_pattern = Pattern::from_render_target(contents.pattern.source.render_target_id,
                                                                   contents.pattern.source.size);
                    //                            new_pattern.set_filter(pattern.filter());
                    new_pattern.apply_transform(transform * contents.pattern.transform);
                    new_pattern.set_repeat_x(contents.pattern.repeat_x());
                    new_pattern.set_repeat_y(contents.pattern.repeat_y());
                    new_pattern.set_smoothing_enabled(contents.pattern.smoothing_enabled());

                    auto new_paint = Paint::from_pattern(new_pattern);
                    push_paint(new_paint);
                } else {
                    new_paint_id = push_paint(paint);
                }
            } else {
                contents.gradient.geometry.apply_transform(transform);
                new_paint_id = push_paint(paint);
            }
        } else {
            new_paint_id = push_paint(paint);
        }

        paint_mapping.insert({old_paint_id, new_paint_id});
    }

    return {render_target_mapping, paint_mapping};
}

void Palette::allocate_textures(const std::shared_ptr<PaintTextureManager> &texture_manager, Renderer *renderer) {
    auto &allocator = texture_manager->allocator;
    auto iter = allocator.page_ids();

    while (true) {
        auto page_id = iter.next();

        if (page_id != nullptr) {
            auto page_size = allocator.page_size(*page_id);

            if (allocator.page_is_new(*page_id)) {
                renderer->allocate_pattern_texture_page(*page_id, page_size);
            }
        } else {
            break;
        }
    }

    allocator.mark_all_pages_as_allocated();
}

} // namespace Pathfinder
