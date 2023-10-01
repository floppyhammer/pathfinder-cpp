#ifndef PATHFINDER_RENDERER_H
#define PATHFINDER_RENDERER_H

#include <cstdint>

#include "../gpu/device.h"
#include "../gpu/queue.h"
#include "../gpu_mem/allocator.h"
#include "data/data.h"
#include "scene_builder.h"

namespace Pathfinder {

/// Mask tile dimension.
const uint32_t MASK_TILES_ACROSS = 256;
const uint32_t MASK_TILES_DOWN = 256;

/// Mask framebuffer size.
// Divide the height by 4 to compress the rows into rgba channels.
const uint32_t MASK_FRAMEBUFFER_WIDTH = TILE_WIDTH * MASK_TILES_ACROSS;
const uint32_t MASK_FRAMEBUFFER_HEIGHT = TILE_HEIGHT / 4 * MASK_TILES_DOWN;

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

struct MaskStorage {
    /// For Dx9.
    std::shared_ptr<uint64_t> framebuffer_id;
    /// For Dx11.
    std::shared_ptr<uint64_t> texture_id;
    uint32_t allocated_page_count = 0;
};

enum class RenderLevel {
    Dx9,
    Dx11,
};

/// In most cases, we have only one renderer set up, while having
/// multiple scenes prepared for rendering.
/// All GPU operations happens in the renderer.
class Renderer {
public:
    explicit Renderer(const std::shared_ptr<Device> &_device, const std::shared_ptr<Queue> &_queue);

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

    virtual void draw(const std::shared_ptr<SceneBuilder> &_scene_builder, bool _clear_dst_texture) = 0;

    TextureLocation get_render_target_location(RenderTargetId render_target_id);

    RenderTarget get_render_target(RenderTargetId render_target_id);

    void reset();

    std::shared_ptr<Sampler> get_or_create_sampler(TextureSamplingFlags sampling_flags);

    std::shared_ptr<Sampler> get_default_sampler();

    std::shared_ptr<Device> device;

    std::shared_ptr<Queue> queue;

protected:
    virtual TextureFormat mask_texture_format() const = 0;

protected:
    /// If we should clear the dest framebuffer or texture.
    bool clear_dest_texture = true;

    uint32_t alpha_tile_count = 0;

    /// Where to draw the alpha tile mask.
    MaskStorage mask_storage;

    // Basic data.
    std::shared_ptr<GpuMemoryAllocator> allocator;

    // Read-only static core resources.
    // -----------------------------------------------
    /// Uniform buffer containing some constants. Shared by D3D9 and D3D10.
    uint64_t common_sizes_ub_id;

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
