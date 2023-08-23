#include "renderer.h"

#include <umHalf.h>

#include <array>

#include "../common/io.h"
#include "../shaders/generated/area_lut_png.h"
#include "paint/palette.h"

namespace Pathfinder {

Renderer::Renderer(const std::shared_ptr<Device> &_device, const std::shared_ptr<Queue> &_queue)
    : device(_device), queue(_queue) {
    allocator = std::make_shared<GpuMemoryAllocator>(device);

    // Area-Lut texture.
    auto image_buffer = ImageBuffer::from_memory({std::begin(area_lut_png), std::end(area_lut_png)}, false);

    area_lut_texture_id =
        allocator->allocate_texture(image_buffer->get_size(), TextureFormat::Rgba8Unorm, "Area-lut texture");

    // Dummy texture.
    dummy_texture_id = allocator->allocate_texture({1, 1}, TextureFormat::Rgba8Unorm, "Dummy texture");

    metadata_texture_id = allocator->allocate_texture({TEXTURE_METADATA_TEXTURE_WIDTH, TEXTURE_METADATA_TEXTURE_HEIGHT},
                                                      TextureFormat::Rgba16Float,
                                                      "Metadata texture");

    auto encoder = device->create_command_encoder("Upload common renderer data");

    encoder->write_texture(allocator->get_texture(area_lut_texture_id), {}, image_buffer->get_data());

    queue->submit_and_wait(encoder);
}

Renderer::~Renderer() {
    for (const auto &texture_page : pattern_texture_pages) {
        if (texture_page != nullptr) {
            allocator->free_framebuffer(texture_page->framebuffer_id);
        }
    }
}

void Renderer::allocate_pattern_texture_page(uint64_t page_id, Vec2I texture_size) {
    // Fill in IDs up to the requested page ID.
    while (pattern_texture_pages.size() < page_id + 1) {
        pattern_texture_pages.push_back(nullptr);
    }

    // Clear out any existing texture.
    auto old_texture_page = pattern_texture_pages[page_id];
    if (old_texture_page != nullptr) {
        pattern_texture_pages[page_id] = nullptr;
        allocator->free_framebuffer(old_texture_page->framebuffer_id);
    }

    // Allocate texture.
    auto framebuffer_id = allocator->allocate_framebuffer(texture_size, TextureFormat::Rgba8Unorm, "Pattern page");
    pattern_texture_pages[page_id] = std::make_shared<PatternTexturePage>(framebuffer_id, false);
}

void Renderer::declare_render_target(RenderTargetId render_target_id, TextureLocation location) {
    while (render_target_locations.size() < render_target_id.render_target + 1) {
        render_target_locations.push_back(TextureLocation{std::numeric_limits<uint32_t>::max(), RectI()});
    }

    auto &render_target = render_target_locations[render_target_id.render_target];
    assert(render_target.page == std::numeric_limits<uint32_t>::max());
    render_target = location;
}

TextureLocation Renderer::get_render_target_location(RenderTargetId render_target_id) {
    return render_target_locations[render_target_id.render_target];
}

RenderTarget Renderer::get_render_target(RenderTargetId render_target_id) {
    auto texture_page_id = get_render_target_location(render_target_id).page;

    auto texture_page = pattern_texture_pages[texture_page_id];
    if (texture_page == nullptr) {
        Logger::error("Texture page not allocated!", "Renderer");
        return {nullptr};
    }

    return {allocator->get_framebuffer(texture_page->framebuffer_id)};
}

void Renderer::upload_texel_data(std::vector<ColorU> &texels, TextureLocation location) {
    if (location.page >= pattern_texture_pages.size()) {
        Logger::error("Texture page ID is invalid!", "Renderer");
        return;
    }

    auto texture_page = pattern_texture_pages[location.page];
    if (texture_page == nullptr) {
        Logger::error("Texture page not allocated!", "Renderer");
        return;
    }

    auto framebuffer = allocator->get_framebuffer(texture_page->framebuffer_id);
    auto texture = framebuffer->get_texture();

    auto encoder = device->create_command_encoder("Upload data of the pattern texture pages");
    encoder->write_texture(texture, location.rect, texels.data());
    queue->submit_and_wait(encoder);

    texture_page->must_preserve_contents = true;
}

void Renderer::reset() {
    render_target_locations.clear();
    allocator->purge_if_needed();
}

std::shared_ptr<Sampler> Renderer::get_or_create_sampler(TextureSamplingFlags sampling_flags) {
    SamplerDescriptor descriptor{};

    if (sampling_flags.contains(TextureSamplingFlags::REPEAT_U)) {
        descriptor.address_mode_u = SamplerAddressMode::Repeat;
    } else {
        descriptor.address_mode_u = SamplerAddressMode::ClampToEdge;
    }
    if (sampling_flags.contains(TextureSamplingFlags::REPEAT_V)) {
        descriptor.address_mode_v = SamplerAddressMode::Repeat;
    } else {
        descriptor.address_mode_v = SamplerAddressMode::ClampToEdge;
    }
    if (sampling_flags.contains(TextureSamplingFlags::NEAREST_MAG)) {
        descriptor.mag_filter = SamplerFilter::Nearest;
    } else {
        descriptor.mag_filter = SamplerFilter::Linear;
    }
    if (sampling_flags.contains(TextureSamplingFlags::NEAREST_MIN)) {
        descriptor.min_filter = SamplerFilter::Nearest;
    } else {
        descriptor.min_filter = SamplerFilter::Linear;
    }

    for (auto &s : samplers) {
        if (s->get_descriptor() == descriptor) {
            return s;
        }
    }

    auto new_sampler = device->create_sampler(descriptor);
    samplers.push_back(new_sampler);

    return new_sampler;
}

std::shared_ptr<Sampler> Renderer::get_default_sampler() {
    TextureSamplingFlags flags;

    // Note: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
    flags.value = 0;

    return get_or_create_sampler(TextureSamplingFlags{});
}

void Renderer::upload_texture_metadata(const std::vector<TextureMetadataEntry> &metadata) {
    if (metadata.empty()) {
        return;
    }

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
        RectI(0, 0, TEXTURE_METADATA_TEXTURE_WIDTH, texels.size() / (4 * TEXTURE_METADATA_TEXTURE_WIDTH));

    // Don't use a vector as we need to delay the de-allocation until the image data is uploaded to GPU.
    auto raw_texels = new half[texels.size()];
    std::copy(texels.begin(), texels.end(), raw_texels);

    // Callback to clean up staging resources.
    auto callback = [raw_texels] { delete[] raw_texels; };

    auto encoder = device->create_command_encoder("Upload to metadata texture");
    encoder->add_callback(callback);
    encoder->write_texture(allocator->get_texture(metadata_texture_id), region_rect, raw_texels);
    queue->submit_and_wait(encoder);
}

} // namespace Pathfinder
