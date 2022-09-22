#include "renderer.h"

#include <umHalf.h>

#include <array>

#include "../common/io.h"
#include "../common/math/basic.h"
#include "../gpu/command_buffer.h"
#include "../gpu/platform.h"
#include "../shaders/generated/area_lut_png.h"

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

Renderer::Renderer(const std::shared_ptr<Driver> &p_driver) {
    driver = p_driver;
}

void Renderer::set_up() {
    // We only allocate the metadata texture once.
    metadata_texture =
        driver->create_texture(TEXTURE_METADATA_TEXTURE_WIDTH, TEXTURE_METADATA_TEXTURE_HEIGHT, TextureFormat::RGBA16F);

    auto cmd_buffer = driver->create_command_buffer(true);

    // Uniform buffer.
    {
        fixed_sizes_ub =
            driver->create_buffer(BufferType::Uniform, 8 * sizeof(float), MemoryProperty::HOST_VISIBLE_AND_COHERENT);

        // Upload data to the uniform buffer with fixed data.
        std::array<float, 6> fixed_sizes_ubo_data = {MASK_FRAMEBUFFER_WIDTH,
                                                     MASK_FRAMEBUFFER_HEIGHT,
                                                     TILE_WIDTH,
                                                     TILE_HEIGHT,
                                                     TEXTURE_METADATA_TEXTURE_WIDTH,
                                                     TEXTURE_METADATA_TEXTURE_HEIGHT};

        cmd_buffer->upload_to_buffer(fixed_sizes_ub, 0, 6 * sizeof(float), fixed_sizes_ubo_data.data());
    }

    auto image_data = ImageData::from_memory({std::begin(area_lut_png), std::end(area_lut_png)}, false);

    area_lut_texture = driver->create_texture(image_data->width, image_data->height, TextureFormat::RGBA8_UNORM);

    cmd_buffer->upload_to_texture(area_lut_texture, {}, image_data->data, TextureLayout::SHADER_READ_ONLY);

    cmd_buffer->submit(driver);
}

struct FilterParams {
    F32x4 p0 = F32x4::splat(0);
    F32x4 p1 = F32x4::splat(0);
    F32x4 p2 = F32x4::splat(0);
    F32x4 p3 = F32x4::splat(0);
    F32x4 p4 = F32x4::splat(0);
    int32_t ctrl = 0;
};

FilterParams compute_filter_params(const Filter &filter, BlendMode blend_mode, ColorCombineMode color_0_combine_mode) {
    int32_t ctrl = 0;
    ctrl |= blend_mode_to_composite_ctrl(blend_mode) << COMBINER_CTRL_COMPOSITE_SHIFT;
    ctrl |= color_combine_mode_to_composite_ctrl(color_0_combine_mode) << COMBINER_CTRL_COLOR_COMBINE_SHIFT;

    FilterParams filter_params;

    switch (filter.type) {
        case Filter::Type::RadialGradient: {
            filter_params.p0 = F32x4(filter.gradient_filter.line.from(), filter.gradient_filter.line.vector());
            filter_params.p1 = F32x4(filter.gradient_filter.radii, filter.gradient_filter.uv_origin);

            filter_params.ctrl = ctrl | (COMBINER_CTRL_FILTER_RADIAL_GRADIENT << COMBINER_CTRL_COLOR_FILTER_SHIFT);
        } break;
        case Filter::Type::PatternFilter: {
            if (filter.pattern_filter.type == PatternFilter::Type::Blur) {
                auto sigma = filter.pattern_filter.blur.sigma;
                auto direction = filter.pattern_filter.blur.direction;

                if (sigma <= 0) break;

                auto sigma_inv = 1.f / sigma;
                auto gauss_coeff_x = SQRT_2_PI_INV * sigma_inv;
                auto gauss_coeff_y = std::exp(-0.5f * sigma_inv * sigma_inv);
                auto gauss_coeff_z = gauss_coeff_y * gauss_coeff_y;

                auto src_offset = direction == BlurDirection::X ? Vec2<float>(1.0, 0.0) : Vec2<float>(0.0, 1.0);

                auto support = std::ceil(1.5f * sigma) * 2.f;

                filter_params.p0 = F32x4(src_offset, Vec2<float>(support, 0.0));
                filter_params.p1 = F32x4(gauss_coeff_x, gauss_coeff_y, gauss_coeff_z, 0.0);
                filter_params.ctrl = ctrl | (COMBINER_CTRL_FILTER_BLUR << COMBINER_CTRL_COLOR_FILTER_SHIFT);
            }
        } break;
        default: {
            filter_params.ctrl = ctrl;
        } break;
    }

    return filter_params;
}

void upload_metadata(const std::shared_ptr<Texture> &metadata_texture,
                     const std::vector<TextureMetadataEntry> &metadata,
                     const std::shared_ptr<Driver> &driver) {
    auto padded_texel_size =
        alignup_i32((int32_t)metadata.size(), TEXTURE_METADATA_ENTRIES_PER_ROW) * TEXTURE_METADATA_TEXTURE_WIDTH * 4;

    std::vector<half> texels;
    texels.reserve(padded_texel_size);

    for (const auto &entry : metadata) {
        auto base_color = entry.base_color.to_f32();

        auto filter_params = compute_filter_params(entry.filter, entry.blend_mode, entry.color_0_combine_mode);

        std::array<half, 40> slice = {
            // 0
            entry.color_0_transform.m11(),
            entry.color_0_transform.m21(),
            entry.color_0_transform.m12(),
            entry.color_0_transform.m22(),
            // 1
            entry.color_0_transform.m13(),
            entry.color_0_transform.m23(),
            0.0f,
            0.0f,
            // 2
            base_color.r,
            base_color.g,
            base_color.b,
            base_color.a,
            // 3
            filter_params.p0.xy().x,
            filter_params.p0.xy().y,
            filter_params.p0.zw().x,
            filter_params.p0.zw().y,
            // 4
            filter_params.p1.xy().x,
            filter_params.p1.xy().y,
            filter_params.p1.zw().x,
            filter_params.p1.zw().y,
            // 5
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            // 6
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            // 7
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            // 8
            (float)filter_params.ctrl,
            0.0f,
            0.0f,
            0.0f,
            // 9
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
    cmd_buffer->upload_to_texture(metadata_texture, region_rect, raw_texels, TextureLayout::SHADER_READ_ONLY);
    cmd_buffer->submit(driver);
}
} // namespace Pathfinder
