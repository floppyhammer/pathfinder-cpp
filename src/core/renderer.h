#ifndef PATHFINDER_RENDERER_H
#define PATHFINDER_RENDERER_H

#include <cstdint>

#include "../gpu/device.h"
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
                             const std::shared_ptr<Device> &device);

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
/// All GPU operations happens in the renderer.
class Renderer {
public:
    explicit Renderer(const std::shared_ptr<Device> &_device);

    ~Renderer();

    /// Upload texture metadata built by palette.
    void upload_texture_metadata(const std::vector<TextureMetadataEntry> &metadata);

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

    /// Performs work necessary to begin rendering a scene.
    ///
    /// This must be called before `render_command()`.
    void begin_scene();

    /// Finishes rendering a scene.
    ///
    /// Note that, after calling this method, you might need to flush the output to the screen via
    /// `swap_buffers()`, `present()`, or a similar method that your windowing library offers.
    void end_scene();

    std::shared_ptr<Sampler> get_or_create_sampler(TextureSamplingFlags sampling_flags);

    std::shared_ptr<Sampler> get_default_sampler();

    std::shared_ptr<Device> device;

protected:
    /// If we should clear the dest framebuffer or texture.
    bool clear_dest_texture = true;

    // Basic data.
    std::shared_ptr<GpuMemoryAllocator> allocator;

    // Read-only static core resources.
    // -----------------------------------------------
    /// Uniform buffer containing some constants. Shared by D3D9 and D3D10.
    uint64_t constants_ub_id;

    /// Pre-Defined texture used to draw the mask texture. Shared by D3D9 and D3D10.
    uint64_t area_lut_texture_id;

    /// For unused texture binding point.
    uint64_t dummy_texture_id;
    // -----------------------------------------------

    // Read-write static core resources.
    // -----------------------------------------------
    uint64_t metadata_texture_id;
    // -----------------------------------------------

    // Dynamic resources and associated metadata.
    // -----------------------------------------------
    std::vector<TextureLocation> render_target_locations;
    std::vector<std::shared_ptr<PatternTexturePage>> pattern_texture_pages;
    // -----------------------------------------------

    std::vector<std::shared_ptr<Sampler>> samplers;
};

} // namespace Pathfinder

#endif // PATHFINDER_RENDERER_H
