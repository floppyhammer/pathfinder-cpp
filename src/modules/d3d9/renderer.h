//
// Created by floppyhammer on 6/19/2021.
//

#ifndef PATHFINDER_D3D9_RENDERER_H
#define PATHFINDER_D3D9_RENDERER_H

#include "object_builder.h"
#include "../../rendering/raster_program.h"
#include "../../rendering/viewport.h"
#include "../../rendering/texture.h"
#include "../d3d9_d3d11/renderer.h"
#include "../../rendering/render_pipeline.h"
#include "../../rendering/descriptor_set.h"

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

        explicit RendererD3D9(const Vec2<int>& p_viewport_size);

        void set_up_pipelines();

        void draw(const SceneBuilderD3D9& scene_builder);

    private:
        /// Fill shader program.
        std::shared_ptr<RasterProgram> fill_program;
        /// Tile shader program.
        std::shared_ptr<RasterProgram> tile_program;

        /// VAOs and VBOs.
        unsigned int quad_vbo{};
        unsigned int fill_vbo{}, fill_vao{};
        unsigned int tile_vbo{}, tile_vao{};

        std::shared_ptr<Buffer> quad_vertex_buffer, fill_vertex_buffer, tile_vertex_buffer;

        std::shared_ptr<RenderPipeline> fill_pipeline, tile_pipeline;
        std::shared_ptr<DescriptorSet> fill_descriptor_set, tile_descriptor_set;

        /// Uniform buffers.
        std::shared_ptr<Buffer> tile_varying_sizes_ub{}, tile_transform_ub{};

        std::shared_ptr<Viewport> mask_viewport;

        void upload_and_draw_tiles(const std::vector<DrawTileBatch>& tile_batches,
                                   const std::vector<TextureMetadataEntry>& metadata);

        /// Upload fills data to GPU.
        void upload_fills(const std::vector<Fill>& fills);

        /// Upload tiles data to GPU.
        void upload_tiles(const std::vector<TileObjectPrimitive> &tiles);

        /// Draw tiles.
        void draw_tiles(uint32_t tile_count,
                        const RenderTarget& target_viewport,
                        const RenderTarget& color_texture,
                        const std::shared_ptr<Texture> &z_buffer_texture) const;

        /// Draw the mask texture. Use Renderer::buffered_fills.
        void draw_fills(uint32_t fills_count);
    };
}

#endif //PATHFINDER_D3D9_RENDERER_H
