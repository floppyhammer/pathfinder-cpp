#include "renderer.h"

#include <umHalf.h>

#include <array>

#include "../common/io.h"
#include "../shaders/generated/area_lut_png.h"
#include "paint/palette.h"

namespace Pathfinder {

Renderer::Renderer(const std::shared_ptr<Driver> &_driver) : driver(_driver) {
    allocator = std::make_shared<GpuMemoryAllocator>(driver);

    // Uniform buffer for some constants.
    constants_ub = driver->create_buffer(
        {BufferType::Uniform, 8 * sizeof(float), MemoryProperty::HostVisibleAndCoherent, "Constants uniform buffer"});

    std::array<float, 6> constants = {MASK_FRAMEBUFFER_WIDTH,
                                      MASK_FRAMEBUFFER_HEIGHT,
                                      TILE_WIDTH,
                                      TILE_HEIGHT,
                                      TEXTURE_METADATA_TEXTURE_WIDTH,
                                      TEXTURE_METADATA_TEXTURE_HEIGHT};

    // Area-Lut texture.
    auto image_buffer = ImageBuffer::from_memory({std::begin(area_lut_png), std::end(area_lut_png)}, false);

    area_lut_texture =
        driver->create_texture({image_buffer->get_size(), TextureFormat::Rgba8Unorm, "Area-Lut texture"});

    // Dummy texture.
    dummy_texture = driver->create_texture({{1, 1}, TextureFormat::Rgba8Unorm, "Dummy texture"});

    metadata_texture = driver->create_texture({{TEXTURE_METADATA_TEXTURE_WIDTH, TEXTURE_METADATA_TEXTURE_HEIGHT},
                                               TextureFormat::Rgba16Float,
                                               "Metadata texture"});

    auto cmd_buffer = driver->create_command_buffer("Upload constant data");

    cmd_buffer->upload_to_buffer(constants_ub, 0, 6 * sizeof(float), constants.data());

    cmd_buffer->upload_to_texture(area_lut_texture, {}, image_buffer->get_data());

    cmd_buffer->submit_and_wait();
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
    auto framebuffer_id = allocator->allocate_framebuffer(texture_size, TextureFormat::Rgba8Unorm, "PatternPage");
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
        Logger::error("Texture page not allocated yet!", "Renderer");
        return;
    }

    auto framebuffer = allocator->get_framebuffer(texture_page->framebuffer_id);
    auto texture = framebuffer->get_texture();

    auto cmd_buffer = driver->create_command_buffer("Upload data of the pattern texture pages");
    cmd_buffer->upload_to_texture(texture, location.rect, texels.data());
    cmd_buffer->submit_and_wait();

    texture_page->must_preserve_contents = true;
}

void Renderer::start_rendering() {
    render_target_locations.clear();
}

} // namespace Pathfinder
