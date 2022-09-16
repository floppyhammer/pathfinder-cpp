#ifndef PATHFINDER_D3D9_RENDERER_H
#define PATHFINDER_D3D9_RENDERER_H

#include "object_builder.h"
#include "../renderer.h"
#include "../../common/global_macros.h"
#include "../../gpu/framebuffer.h"
#include "../../gpu/texture.h"
#include "../../gpu/render_pass.h"
#include "../../gpu/render_pipeline.h"
#include "../../gpu/descriptor_set.h"

#include <vector>

namespace Pathfinder {
    /// Renderer should run in a different thread than that of the builder.
    class RendererD3D9 : public Renderer {
    public:
        /// Fills to draw.
        std::vector<Fill> buffered_fills;
        std::vector<Fill> pending_fills;

        /// Tiles to draw.
        std::vector<DrawTileBatch> pending_tile_batches;

        explicit RendererD3D9(const std::shared_ptr<Driver> &p_driver);

        void set_up_pipelines() override;

        /// We need to call this for each scene.
        void draw(const std::shared_ptr<SceneBuilder> &p_scene_builder) override;

        std::shared_ptr<Texture> get_dest_texture() override;

        void set_dest_texture(const std::shared_ptr<Texture> &texture) override;

    private:
        /// Vertex buffers.
        std::shared_ptr<Buffer> quad_vertex_buffer, fill_vertex_buffer, tile_vertex_buffer;

        /// Pipelines.
        std::shared_ptr<RenderPipeline> fill_pipeline, tile_pipeline;

        /// Descriptor sets.
        std::shared_ptr<DescriptorSet> fill_descriptor_set, tile_descriptor_set;

        /// Uniform buffers.
        std::shared_ptr<Buffer> tile_varying_sizes_ub{}, tile_transform_ub{};

        /// Where the final rendering output goes.
        std::shared_ptr<Framebuffer> dest_framebuffer;

        /// Where to draw the mask texture.
        std::shared_ptr<Framebuffer> mask_framebuffer;

        std::shared_ptr<RenderPass> mask_render_pass, dest_render_pass_clear, dest_render_pass_load;

        void upload_and_draw_tiles(const std::vector<DrawTileBatch> &tile_batches,
                                   const std::vector<TextureMetadataEntry> &metadata);

        /// Upload fills data to GPU.
        void upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandBuffer> &cmd_buffer);

        /// Upload tiles data to GPU.
        void
        upload_tiles(const std::vector<TileObjectPrimitive> &tiles, const std::shared_ptr<CommandBuffer> &cmd_buffer);

        /// Draw tiles.
        void draw_tiles(uint32_t tile_count,
                        const RenderTarget &target_viewport,
                        const std::shared_ptr<Texture> &color_texture,
                        const std::shared_ptr<Texture> &z_buffer_texture,
                        const std::shared_ptr<CommandBuffer> &cmd_buffer);

        /// Draw the mask texture. Use Renderer::buffered_fills.
        void draw_fills(uint32_t fills_count, const std::shared_ptr<CommandBuffer> &cmd_buffer);
    };
}

#endif //PATHFINDER_D3D9_RENDERER_H
