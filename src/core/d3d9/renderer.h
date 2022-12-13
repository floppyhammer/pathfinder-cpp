#ifndef PATHFINDER_D3D9_RENDERER_H
#define PATHFINDER_D3D9_RENDERER_H

#include <vector>

#include "../../common/global_macros.h"
#include "../../gpu/descriptor_set.h"
#include "../../gpu/framebuffer.h"
#include "../../gpu/render_pass.h"
#include "../../gpu/render_pipeline.h"
#include "../../gpu/texture.h"
#include "../renderer.h"
#include "object_builder.h"

namespace Pathfinder {

struct ClipBufferInfo {
    std::shared_ptr<Buffer> clip_buffer;
    uint32_t clip_count;
};

/// Renderer should run in a different thread than that of the builder.
class RendererD3D9 : public Renderer {
public:
    /// Fills to draw.
    std::vector<Fill> buffered_fills;
    std::vector<Fill> pending_fills;

    /// Tiles to draw.
    std::vector<DrawTileBatchD3D9> pending_tile_batches;

    explicit RendererD3D9(const std::shared_ptr<Driver> &_driver);

    void set_up_pipelines() override;

    /// We need to call this for each scene.
    void draw(const std::shared_ptr<SceneBuilder> &_scene_builder) override;

    std::shared_ptr<Texture> get_dest_texture() override;

    void set_dest_texture(const std::shared_ptr<Texture> &new_texture) override;

private:
    /// Vertex buffers.
    std::shared_ptr<Buffer> quad_vertex_buffer;                     // Static
    std::shared_ptr<Buffer> fill_vertex_buffer, tile_vertex_buffer; // Dynamic

    /// Pipelines.
    std::shared_ptr<RenderPipeline> fill_pipeline, tile_pipeline;
    std::shared_ptr<RenderPipeline> tile_clip_copy_pipeline, tile_clip_combine_pipeline; // For clip paths.

    /// Descriptor sets.
    std::shared_ptr<DescriptorSet> fill_descriptor_set, tile_descriptor_set;
    std::shared_ptr<DescriptorSet> tile_clip_copy_descriptor_set, tile_clip_combine_descriptor_set; // For clip paths.

    /// Uniform buffers.
    std::shared_ptr<Buffer> tile_varying_sizes_ub, tile_transform_ub;
    std::shared_ptr<Buffer> tile_clip_copy_ub; // For clip paths.

    /// Where the final rendering output goes.
    std::shared_ptr<Framebuffer> dest_framebuffer;

    // Z buffer texture cache. So we don't have to recreate the texture for every batch and frame.
    std::shared_ptr<Texture> z_buffer_texture;

    /// Where to draw the mask texture.
    std::shared_ptr<Framebuffer> mask_framebuffer, temp_mask_framebuffer;

    std::shared_ptr<RenderPass> mask_render_pass_clear, mask_render_pass_load, dest_render_pass_clear,
        dest_render_pass_load;

private:
    void create_tile_clip_copy_pipeline();

    void create_tile_clip_combine_pipeline();

    void upload_and_draw_tiles(const std::vector<DrawTileBatchD3D9> &tile_batches);

    /// Upload fills data to GPU.
    void upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    ClipBufferInfo upload_clip_tiles(const std::vector<Clip> &clips, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Apply clip paths.
    void clip_tiles(const ClipBufferInfo &clip_buffer_info, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    void upload_z_buffer(const DenseTileMap<uint32_t> &z_buffer_map, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Upload tiles data to GPU.
    void upload_tiles(const std::vector<TileObjectPrimitive> &tiles, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Draw tiles.
    void draw_tiles(uint32_t tile_count,
                    const RenderTarget &target_viewport,
                    const std::shared_ptr<Texture> &metadata_texture,
                    const std::shared_ptr<Texture> &color_texture,
                    const std::shared_ptr<Texture> &z_buffer_texture,
                    const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Draw the mask texture. Use Renderer::buffered_fills.
    void draw_fills(uint32_t fills_count, const std::shared_ptr<CommandBuffer> &cmd_buffer);
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_RENDERER_H
