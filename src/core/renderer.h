#ifndef PATHFINDER_RENDERER_H
#define PATHFINDER_RENDERER_H

#include <cstdint>

#include "../gpu/driver.h"
#include "../gpu_mem/allocator.h"
#include "data/data.h"
#include "scene_builder.h"

namespace Pathfinder {

/// Mask tile dimension.
const uint32_t MASK_TILES_ACROSS = 256;
const uint32_t MASK_TILES_DOWN = 256;

/// Mask framebuffer size.
// Divide the height by 4 to compress the rows into rgba channels.
const int32_t MASK_FRAMEBUFFER_WIDTH = TILE_WIDTH * MASK_TILES_ACROSS;
const int32_t MASK_FRAMEBUFFER_HEIGHT = TILE_HEIGHT / 4 * MASK_TILES_DOWN;

struct RenderTarget {
    std::shared_ptr<Framebuffer> framebuffer;
};

void upload_texture_metadata(const std::shared_ptr<Texture> &metadata_texture,
                             const std::vector<TextureMetadataEntry> &metadata,
                             const std::shared_ptr<Driver> &driver);

/// Pattern GPU textures.
class PatternTexturePage {
public:
    PatternTexturePage(uint64_t _framebuffer_id, bool _must_preserve_contents)
        : framebuffer_id(_framebuffer_id), must_preserve_contents(_must_preserve_contents) {}

    uint64_t framebuffer_id;
    /// Should preserve framebuffer content.
    bool must_preserve_contents;
};

/// In most cases, we have only one renderer set up, while having
/// multiple scenes prepared for rendering.
class Renderer {
public:
    explicit Renderer(const std::shared_ptr<Driver> &_driver);

    ~Renderer();

    /// Allocate GPU resources for a pattern texture page.
    void allocate_pattern_texture_page(uint64_t page_id, Vec2I texture_size);

    void upload_texel_data(std::vector<ColorU> &texels, TextureLocation location);

    void declare_render_target(RenderTargetId render_target_id, TextureLocation location);

    virtual void set_up_pipelines() = 0;

    virtual std::shared_ptr<Texture> get_dest_texture() = 0;

    virtual void set_dest_texture(const std::shared_ptr<Texture> &new_texture) = 0;

    void start_rendering();

    virtual void draw(const std::shared_ptr<SceneBuilder> &_scene_builder) = 0;

    TextureLocation get_render_target_location(RenderTargetId render_target_id);

    RenderTarget get_render_target(RenderTargetId render_target_id);

    std::shared_ptr<Driver> driver;

    std::shared_ptr<Texture> metadata_texture;

    std::vector<std::shared_ptr<PatternTexturePage>> pattern_texture_pages;

protected:
    /// If we should clear the dest framebuffer or texture.
    bool clear_dest_texture = true;

    /// Pre-Defined texture used to draw the mask texture. Shared by D3D9 and D3D10.
    std::shared_ptr<Texture> area_lut_texture;

    /// For unused texture binding point.
    std::shared_ptr<Texture> dummy_texture;

    /// Uniform buffer containing some constants. Shared by D3D9 and D3D10.
    std::shared_ptr<Buffer> constants_ub{};

    std::shared_ptr<GpuMemoryAllocator> allocator;

    std::vector<TextureLocation> render_target_locations;
};

} // namespace Pathfinder

#endif // PATHFINDER_RENDERER_H
