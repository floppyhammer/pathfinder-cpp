//
// Created by floppyhammer on 6/25/2021.
//

#include "renderer.h"

#include "../../rendering/device_gl.h"
#include "../../common/timestamp.h"
#include "../../common/math/basic.h"
#include "../../common/global_macros.h"
#include "../../common/math/vec3.h"
#include "../../common/math/mat4x4.h"

namespace Pathfinder {
    static uint16_t QUAD_VERTEX_POSITIONS[12] = {0, 0, 1, 0, 0, 1,
                                                 1, 0, 1, 1, 0, 1};

    const size_t FILL_INSTANCE_SIZE = 12;
    const size_t CLIP_TILE_INSTANCE_SIZE = 16;

    // 65536
    const size_t MAX_FILLS_PER_BATCH = 0x10000;

    /// It might not be worth it to cache z buffer textures as they're generally small.
    std::shared_ptr<Texture> upload_z_buffer(const DenseTileMap<uint32_t> &z_buffer_map) {
        auto z_buffer_texture = std::make_shared<Texture>(
                z_buffer_map.rect.width(),
                z_buffer_map.rect.height(),
                TextureFormat::RGBA8,
                DataType::UNSIGNED_BYTE,
                z_buffer_map.data.data());

        return z_buffer_texture;
    }

    RendererD3D9::RendererD3D9(const Vec2<int> &p_viewport_size) : Renderer(p_viewport_size) {
#ifdef PATHFINDER_SHIP_SHADERS
        const std::string fill_vert_source =
#include "../src/shaders/minified/minified_fill.vert"
;

        const std::string fill_frag_source =
#include "../src/shaders/minified/minified_fill.frag"
;

        const std::string tile_vert_source =
#include "../src/shaders/minified/minified_tile.vert"
;

        const std::string tile_frag_source_0 =
#include "../src/shaders/minified/minified_tile.frag.0"
;

        const std::string tile_frag_source_1 =
#include "../src/shaders/minified/minified_tile.frag.1"
;

        fill_program = std::make_shared<RasterProgram>(fill_vert_source,
                                                       fill_frag_source);

        tile_program = std::make_shared<RasterProgram>(tile_vert_source,
                                                       tile_frag_source_0 + tile_frag_source_1);
#else
        fill_program = std::make_shared<RasterProgram>(PATHFINDER_SHADER_DIR"d3d9/fill.vert",
                                                       PATHFINDER_SHADER_DIR"d3d9/fill.frag");

        tile_program = std::make_shared<RasterProgram>(PATHFINDER_SHADER_DIR"d3d9/tile.vert",
                                                       PATHFINDER_SHADER_DIR"d3d9/tile.frag");
#endif
        mask_viewport = std::make_shared<Viewport>(MASK_FRAMEBUFFER_WIDTH,
                                                   MASK_FRAMEBUFFER_HEIGHT,
                                                   TextureFormat::RGBA16F,
                                                   DataType::HALF_FLOAT);

        glGenVertexArrays(1, &fill_vao);
        glGenVertexArrays(1, &tile_vao);

        glGenBuffers(1, &fill_vbo);
        glGenBuffers(1, &tile_vbo);

        // Quad vertex buffer. Shared by fills and tiles drawing.
        glGenBuffers(1, &quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(uint16_t), QUAD_VERTEX_POSITIONS, GL_STATIC_DRAW);

        DeviceGl::check_error("PathfinderD3D9::RendererD3D9() > setup");
    }

    RendererD3D9::~RendererD3D9() {
        // Clean up.
        glDeleteVertexArrays(1, &fill_vao);
        glDeleteVertexArrays(1, &tile_vao);

        glDeleteBuffers(1, &quad_vbo);
        glDeleteBuffers(1, &fill_vbo);
        glDeleteBuffers(1, &tile_vbo);

        DeviceGl::check_error("RendererD3D9::~RendererD3D9() > gl delete");
    }

    void RendererD3D9::draw(const SceneBuilderD3D9 &scene_builder) {
        // TODO: We should do this before the builder finishes building.
        {
            // Upload fills to buffer.
            upload_fills(scene_builder.pending_fills);

            // We can do fill drawing as soon as the fill vertex buffer is ready.
            draw_fills(scene_builder.pending_fills.size());
        }

        // Tiles need to be drawn after fill drawing and after tile batches are prepared.
        upload_and_draw_tiles(scene_builder.tile_batches, scene_builder.metadata);
    }

    void RendererD3D9::upload_fills(const std::vector<Fill> &fills) const {
        DeviceGl::upload_to_vertex_buffer(fill_vbo,
                                          sizeof(Fill) * fills.size(),
                                          (void *) fills.data());

        DeviceGl::check_error("RendererD3D9::upload_fills");
    }

    void RendererD3D9::upload_tiles(const std::vector<TileObjectPrimitive> &tiles) const {
        DeviceGl::upload_to_vertex_buffer(tile_vbo,
                                          sizeof(TileObjectPrimitive) * tiles.size(),
                                          (void *) tiles.data());

        DeviceGl::check_error("RendererD3D9::upload_tiles");
    }

    void RendererD3D9::upload_and_draw_tiles(const std::vector<DrawTileBatch> &tile_batches,
                                             const std::vector<TextureMetadataEntry> &metadata) {
        // Upload metadata (color, blur, etc...).
        upload_metadata(metadata_texture, metadata);

        for (const auto &batch: tile_batches) {
            auto z_buffer_texture = upload_z_buffer(batch.z_buffer_data);

            upload_tiles(batch.tiles);

            draw_tiles(batch.tiles.size(),
                       batch.viewport,
                       batch.color_texture,
                       z_buffer_texture);
        }
    }

    void RendererD3D9::draw_fills(uint32_t fills_count) {
        // No fills to draw or no valid mask viewport.
        if (fills_count == 0 || mask_viewport == nullptr) return;

        // Must clear the mask texture with transparent color, or the final result will show strange horizontal lines.
        FramebufferDescriptor descriptor = {
                mask_viewport->get_framebuffer_id(),
                mask_viewport->get_width(),
                mask_viewport->get_height(),
                GL_ONE, // Blending is a must.
                GL_ONE,
                true,
        };

        DeviceGl::bind_framebuffer(descriptor);

        fill_program->use();

        // Set uniforms.
        fill_program->set_vec2("uFramebufferSize",
                               (float) mask_viewport->get_width(),
                               (float) mask_viewport->get_height());
        fill_program->set_vec2("uTileSize", TILE_WIDTH, TILE_HEIGHT);

        // Bind textures.
        fill_program->bind_texture(0, "uAreaLUT", area_lut_texture->get_texture_id());

        // Set vertex attributes.
        // ---------------------------------------------
        std::vector<AttributeDescriptor> attribute_descriptors;
        attribute_descriptors.reserve(3);

        // Quad vertex.
        attribute_descriptors.push_back({fill_vao,
                                         quad_vbo,
                                         2,
                                         DataType::UNSIGNED_SHORT,
                                         0,
                                         0,
                                         VertexStep::PER_VERTEX});

        // Vertex stride for the second vertex buffer.
        uint32_t stride = sizeof(Fill);

        // LineSegmentU16.
        attribute_descriptors.push_back({fill_vao,
                                         fill_vbo,
                                         4,
                                         DataType::UNSIGNED_SHORT,
                                         stride,
                                         0,
                                         VertexStep::PER_INSTANCE});
        // Link.
        attribute_descriptors.push_back({fill_vao,
                                         fill_vbo,
                                         1,
                                         DataType::UNSIGNED_INT,
                                         stride,
                                         offsetof(Fill, link),
                                         VertexStep::PER_INSTANCE});

        DeviceGl::bind_attributes(attribute_descriptors);
        // ---------------------------------------------

        DeviceGl::draw(6, fills_count);
    }

    void RendererD3D9::draw_tiles(uint32_t tiles_count,
                                  const RenderTarget &target_viewport,
                                  const RenderTarget &color_texture,
                                  const std::shared_ptr<Texture> &z_buffer_texture) const {
        // No tiles to draw.
        if (tiles_count == 0) return;

        // Bind target framebuffer.
        // -----------------------------------------------
        FramebufferDescriptor descriptor;
        descriptor.blend_src = GL_ONE;
        descriptor.blend_dst = GL_ONE_MINUS_SRC_ALPHA;

        // If no specific RenderTarget is given.
        if (target_viewport.framebuffer_id == 0) {
            descriptor.framebuffer_id = dest_viewport->get_framebuffer_id();
            descriptor.width = viewport_size.x;
            descriptor.height = viewport_size.y;
            descriptor.clear = false;
        } else { // Otherwise, we need to render to that render target.
            descriptor.framebuffer_id = target_viewport.framebuffer_id;
            descriptor.width = target_viewport.size.x;
            descriptor.height = target_viewport.size.y;
            descriptor.clear = true;
        }

        DeviceGl::bind_framebuffer(descriptor);
        // -----------------------------------------------

        // MVP.
        auto model_mat = Mat4x4<float>(1.f);
        model_mat = model_mat.translate(Vec3<float>(-1.f, 1.f, 0.f));
        model_mat = model_mat.scale(Vec3<float>(2.f / (float) descriptor.width, -2.f / (float) descriptor.height, 1.f));
        auto mvp_mat = model_mat;

        tile_program->use();

        // Set uniforms.
        tile_program->set_mat4("uTransform", mvp_mat);
        tile_program->set_vec2("uTileSize", 16.0f, 16.0f);
        tile_program->set_vec2("uTextureMetadataSize", (float) metadata_texture->get_width(),
                               (float) metadata_texture->get_height());
        tile_program->set_vec2("uMaskTextureSize0", 4096.0f, 1024.0f);
        tile_program->set_vec2("uFramebufferSize", (float) descriptor.width, (float) descriptor.height);
        tile_program->set_vec2i("uZBufferSize",
                                (int) z_buffer_texture->get_width(),
                                (int) z_buffer_texture->get_height());
        if (color_texture.texture_id != 0) {
            tile_program->set_vec2("uColorTextureSize0", (float) color_texture.size.x, (float) color_texture.size.y);
        } else {
            tile_program->set_vec2("uColorTextureSize0", 1, 1);
        }

        // Bind textures.
        tile_program->bind_texture(0, "uDestTexture", 0);
        tile_program->bind_texture(1, "uTextureMetadata", metadata_texture->get_texture_id());
        tile_program->bind_texture(2, "uZBuffer", z_buffer_texture->get_texture_id());
        tile_program->bind_texture(3, "uMaskTexture0", mask_viewport->get_texture_id());
        tile_program->bind_texture(4, "uColorTexture0", color_texture.texture_id);

        // Set vertex attributes.
        // ---------------------------------------------
        std::vector<AttributeDescriptor> attribute_descriptors;
        attribute_descriptors.reserve(6);

        // Quad vertex.
        attribute_descriptors.push_back({tile_vao,
                                         quad_vbo,
                                         2,
                                         DataType::SHORT,
                                         0,
                                         0,
                                         VertexStep::PER_VERTEX});

        // Vertex stride for the second vertex buffer.
        uint32_t stride = sizeof(TileObjectPrimitive);

        // Other attributes.
        attribute_descriptors.push_back({tile_vao,
                                         tile_vbo,
                                         2,
                                         DataType::SHORT,
                                         stride,
                                         0,
                                         VertexStep::PER_INSTANCE});
        attribute_descriptors.push_back({tile_vao,
                                         tile_vbo,
                                         4,
                                         DataType::UNSIGNED_BYTE,
                                         stride,
                                         offsetof(TileObjectPrimitive, alpha_tile_id),
                                         VertexStep::PER_INSTANCE});
        attribute_descriptors.push_back({tile_vao,
                                         tile_vbo,
                                         2,
                                         DataType::BYTE,
                                         stride,
                                         offsetof(TileObjectPrimitive, ctrl),
                                         VertexStep::PER_INSTANCE});
        attribute_descriptors.push_back({tile_vao,
                                         tile_vbo,
                                         1,
                                         DataType::INT,
                                         stride,
                                         offsetof(TileObjectPrimitive, path_id),
                                         VertexStep::PER_INSTANCE});
        attribute_descriptors.push_back({tile_vao,
                                         tile_vbo,
                                         1,
                                         DataType::UNSIGNED_INT,
                                         stride,
                                         offsetof(TileObjectPrimitive, color),
                                         VertexStep::PER_INSTANCE});

        DeviceGl::bind_attributes(attribute_descriptors);
        // ---------------------------------------------

        DeviceGl::draw(6, tiles_count);
    }
}
