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
    uint64_t clip_buffer_id;
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

private:
    /// Vertex buffers.
    uint64_t quad_vertex_buffer_id; // Static

    /// Pipelines.
    std::shared_ptr<RenderPipeline> fill_pipeline, tile_pipeline;
    std::shared_ptr<RenderPipeline> tile_clip_copy_pipeline, tile_clip_combine_pipeline; // For clip paths.

    /// Descriptor sets.
    std::shared_ptr<DescriptorSet> fill_descriptor_set, tile_descriptor_set;
    std::shared_ptr<DescriptorSet> tile_clip_copy_descriptor_set, tile_clip_combine_descriptor_set; // For clip paths.

    /// Uniform buffers.
    uint64_t tile_varying_sizes_ub_id, tile_transform_ub_id;

    /// Where the final rendering output goes.
    /// This is not managed by the memory allocator.
    std::shared_ptr<Framebuffer> dest_framebuffer;

    /// Where to draw the mask texture.
    uint64_t mask_framebuffer_id;

    std::shared_ptr<RenderPass> mask_render_pass_clear, mask_render_pass_load;
    std::shared_ptr<RenderPass> dest_render_pass_clear, dest_render_pass_load;

public:
    explicit RendererD3D9(const std::shared_ptr<Driver> &_driver);

    void set_up_pipelines() override;

    /// We need to call this for each scene.
    void draw(const std::shared_ptr<SceneBuilder> &_scene_builder) override;

    std::shared_ptr<Texture> get_dest_texture() override;

    void set_dest_texture(const std::shared_ptr<Texture> &new_texture) override;

private:
    void create_tile_clip_copy_pipeline();

    void create_tile_clip_combine_pipeline();

    void upload_and_draw_tiles(const std::vector<DrawTileBatchD3D9> &tile_batches);

    /// Upload fills data to GPU.
    uint64_t upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    ClipBufferInfo upload_clip_tiles(const std::vector<Clip> &clips, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Apply clip paths.
    void clip_tiles(const ClipBufferInfo &clip_buffer_info, const std::shared_ptr<CommandBuffer> &cmd_buffer);

    uint64_t upload_z_buffer(const DenseTileMap<uint32_t> &z_buffer_map,
                             const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Upload tiles data to GPU.
    uint64_t upload_tiles(const std::vector<TileObjectPrimitive> &tiles,
                          const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Draw tiles.
    void draw_tiles(uint64_t tile_vertex_buffer_id,
                    uint32_t tile_count,
                    const std::shared_ptr<RenderTargetId> &render_target_id,
                    const std::shared_ptr<TileBatchTextureInfo> &color_texture_info,
                    uint64_t z_buffer_texture_id,
                    const std::shared_ptr<CommandBuffer> &cmd_buffer);

    /// Draw the mask texture. Use Renderer::buffered_fills.
    void draw_fills(uint64_t fill_vertex_buffer_id,
                    uint32_t fills_count,
                    const std::shared_ptr<CommandBuffer> &cmd_buffer);
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_RENDERER_H
