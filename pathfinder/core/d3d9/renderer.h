#ifndef PATHFINDER_D3D9_RENDERER_H
#define PATHFINDER_D3D9_RENDERER_H

#include <vector>

#include "../../common/global_macros.h"
#include "../../common/math/mat4.h"
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

struct FillUniformDx9 {
    Vec2F tile_size;        // Fixed as (16, 16).
    Vec2F framebuffer_size; // Mask framebuffer size. Dynamic as (4096, 1024 * page_count).
};

struct TileUniformDx9 {
    Vec2F tile_size; // Fixed as (16, 16).
    Vec2F texture_metadata_size;
    Vec2F z_buffer_size;
    Vec2F mask_texture_size;
    Vec2F color_texture_size;
    Vec2F framebuffer_size;
    Mat4 transform;
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
    uint64_t fill_ub_id, tile_ub_id;

    /// Where the final rendering output goes.
    /// This is not managed by the memory allocator.
    std::shared_ptr<Texture> dest_texture;

    std::shared_ptr<RenderPass> mask_render_pass_clear, mask_render_pass_load;
    std::shared_ptr<RenderPass> dest_render_pass_clear, dest_render_pass_load;

public:
    explicit RendererD3D9(const std::shared_ptr<Device> &_device, const std::shared_ptr<Queue> &_queue);

    void set_up_pipelines() override;

    /// We need to call this for each scene.
    void draw(const std::shared_ptr<SceneBuilder> &_scene_builder, bool _clear_dst_texture) override;

    std::shared_ptr<Texture> get_dest_texture() override;

    void set_dest_texture(const std::shared_ptr<Texture> &new_texture) override;

private:
    void reallocate_alpha_tile_pages_if_necessary();

    TextureFormat mask_texture_format() const override;

    void create_tile_clip_copy_pipeline();

    void create_tile_clip_combine_pipeline();

    void upload_and_draw_tiles(const std::vector<DrawTileBatchD3D9> &tile_batches);

    /// Upload fills data to GPU.
    uint64_t upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandEncoder> &encoder);

    ClipBufferInfo upload_clip_tiles(const std::vector<Clip> &clips, const std::shared_ptr<CommandEncoder> &encoder);

    /// Apply clip paths.
    void clip_tiles(const ClipBufferInfo &clip_buffer_info, const std::shared_ptr<CommandEncoder> &encoder);

    uint64_t upload_z_buffer(const DenseTileMap<uint32_t> &z_buffer_map,
                             const std::shared_ptr<CommandEncoder> &encoder);

    /// Upload tiles data to GPU.
    uint64_t upload_tiles(const std::vector<TileObjectPrimitive> &tiles,
                          const std::shared_ptr<CommandEncoder> &encoder);

    /// Draw tiles.
    void draw_tiles(uint64_t tile_vertex_buffer_id,
                    uint32_t tile_count,
                    const std::shared_ptr<RenderTargetId> &render_target_id,
                    const std::shared_ptr<TileBatchTextureInfo> &color_texture_info,
                    uint64_t z_buffer_texture_id,
                    const std::shared_ptr<CommandEncoder> &encoder);

    /// Draw the mask texture. Use Renderer::buffered_fills.
    void draw_fills(uint64_t fill_vertex_buffer_id,
                    uint32_t fills_count,
                    const std::shared_ptr<CommandEncoder> &encoder);
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_RENDERER_H
