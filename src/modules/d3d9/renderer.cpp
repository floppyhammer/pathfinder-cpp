//
// Created by floppyhammer on 6/25/2021.
//

#include "renderer.h"

#include "../../gpu/gl/driver.h"
#include "../../common/timestamp.h"
#include "../../common/math/basic.h"
#include "../../common/global_macros.h"
#include "../../common/math/vec3.h"
#include "../../common/math/mat4x4.h"
#include "../../common/io.h"
#include "../../gpu/gl/command_buffer.h"
#include "../../gpu/platform.h"

#include <array>

namespace Pathfinder {
    static uint16_t QUAD_VERTEX_POSITIONS[12] = {0, 0, 1, 0, 0, 1,
                                                 1, 0, 1, 1, 0, 1};

    const size_t FILL_INSTANCE_SIZE = 12;
    const size_t CLIP_TILE_INSTANCE_SIZE = 16;

    // 65536
    const size_t MAX_FILLS_PER_BATCH = 0x10000;

    /// It might not be worth it to cache z buffer textures as they're generally small.
    std::shared_ptr<Texture> upload_z_buffer(const std::shared_ptr<Driver> &driver,
                                             const DenseTileMap<uint32_t> &z_buffer_map) {
        auto z_buffer_texture = driver->create_texture(
                z_buffer_map.rect.width(),
                z_buffer_map.rect.height(),
                TextureFormat::RGBA8,
                DataType::UNSIGNED_BYTE);

        auto cmd_buffer = driver->create_command_buffer();
        cmd_buffer->upload_to_texture(z_buffer_texture, {}, z_buffer_map.data.data());
        cmd_buffer->submit(driver);

        return z_buffer_texture;
    }

    RendererD3D9::RendererD3D9(const std::shared_ptr<Driver>& p_driver, uint32_t canvas_width, uint32_t canvas_height)
            : Renderer(p_driver) {
        mask_render_pass = driver->create_render_pass(TextureFormat::RGBA16F);

        dest_render_pass = driver->create_render_pass(TextureFormat::RGBA8);

        mask_framebuffer = driver->create_framebuffer(MASK_FRAMEBUFFER_WIDTH,
                                                      MASK_FRAMEBUFFER_HEIGHT,
                                                      TextureFormat::RGBA16F,
                                                      DataType::HALF_FLOAT,
                                                      mask_render_pass);

        dest_framebuffer = driver->create_framebuffer(canvas_width,
                                                      canvas_height,
                                                      TextureFormat::RGBA8,
                                                      DataType::UNSIGNED_BYTE,
                                                      dest_render_pass);

        // Quad vertex buffer. Shared by fills and tiles drawing.
        quad_vertex_buffer = driver->create_buffer(BufferType::Vertex, 12 * sizeof(uint16_t));

        auto cmd_buffer = driver->create_command_buffer();
        cmd_buffer->upload_to_buffer(quad_vertex_buffer, 0, 12 * sizeof(uint16_t), QUAD_VERTEX_POSITIONS);
        cmd_buffer->submit(driver);
    }

    void RendererD3D9::set_up_pipelines() {
        // Fill pipeline.
        {
#ifdef PATHFINDER_SHADERS_EMBEDDED
            const std::string fill_vert_string =
#include "../src/shaders/minified/minified_fill.vert"
            ;

            const std::string fill_frag_string =
#include "../src/shaders/minified/minified_fill.frag"
            ;

            const std::vector<char> fill_vert_source = {fill_vert_string.begin(), fill_vert_string.end()};
            const std::vector<char> fill_frag_source = {fill_frag_string.begin(), fill_frag_string.end()};
#else
            const auto fill_vert_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/fill.vert");
            const auto fill_frag_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/fill.frag");
#endif

            // Set vertex attributes.
            std::vector<VertexInputAttributeDescription> attribute_descriptions;
            {
                attribute_descriptions.reserve(3);

                // Quad vertex.
                attribute_descriptions.push_back({0,
                                                 2,
                                                 DataType::UNSIGNED_SHORT,
                                                 2 * sizeof(uint16_t),
                                                 0,
                                                 VertexInputRate::VERTEX});

                // Vertex stride for the second vertex buffer.
                uint32_t stride = sizeof(Fill);

                // LineSegmentU16.
                attribute_descriptions.push_back({1,
                                                 4,
                                                 DataType::UNSIGNED_SHORT,
                                                 stride,
                                                 0,
                                                 VertexInputRate::INSTANCE});
                // Link.
                attribute_descriptions.push_back({1,
                                                 1,
                                                 DataType::UNSIGNED_INT,
                                                 stride,
                                                 offsetof(Fill, link),
                                                 VertexInputRate::INSTANCE});
            }

            ColorBlendState blend_state = {true, BlendFactor::ONE, BlendFactor::ONE};

            // Set descriptor set.
            {
                fill_descriptor_set = std::make_shared<DescriptorSet>();

                fill_descriptor_set->add_or_update_descriptor({
                    DescriptorType::UniformBuffer,
                    ShaderType::Vertex,
                    0,
                    "bFixedSizes",
                    fixed_sizes_ub,
                    nullptr});
                fill_descriptor_set->add_or_update_descriptor({
                    DescriptorType::Texture,
                    ShaderType::Fragment,
                    0,
                    "uAreaLUT",
                    nullptr,
                    area_lut_texture});
            }

            fill_pipeline = driver->create_render_pipeline(fill_vert_source,
                                                           fill_frag_source,
                                                           attribute_descriptions,
                                                           blend_state,
                                                           fill_descriptor_set,
                                                           mask_render_pass);
        }

        // Tile pipeline.
        {
#ifdef PATHFINDER_SHADERS_EMBEDDED
            const std::string tile_vert_string =
#include "../src/shaders/minified/minified_tile.vert"
            ;

            const std::string tile_frag_string_0 =
#include "../src/shaders/minified/minified_tile.frag.0"
            ;

            const std::string tile_frag_string_1 =
#include "../src/shaders/minified/minified_tile.frag.1"
            ;
            const std::string tile_frag_string = tile_frag_string_0 + tile_frag_string_1;

            const std::vector<char> tile_vert_source = {tile_vert_string.begin(), tile_vert_string.end()};
            const std::vector<char> tile_frag_source = {tile_frag_string.begin(), tile_frag_string.end()};
#else
            const auto tile_vert_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/tile.vert");
            const auto tile_frag_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/tile.frag");
#endif

            // Set vertex attributes.
            std::vector<VertexInputAttributeDescription> attribute_descriptions;
            {
                attribute_descriptions.reserve(6);

                // Quad vertex.
                attribute_descriptions.push_back({0,
                                                 2,
                                                 DataType::UNSIGNED_SHORT,
                                                 0,
                                                 0,
                                                 VertexInputRate::VERTEX});

                // Vertex stride for the second vertex buffer.
                uint32_t stride = sizeof(TileObjectPrimitive);

                // Other attributes.
                attribute_descriptions.push_back({1,
                                                 2,
                                                 DataType::SHORT,
                                                 stride,
                                                 0,
                                                 VertexInputRate::INSTANCE});
                attribute_descriptions.push_back({1,
                                                 4,
                                                 DataType::UNSIGNED_BYTE,
                                                 stride,
                                                 offsetof(TileObjectPrimitive, alpha_tile_id),
                                                 VertexInputRate::INSTANCE});
                attribute_descriptions.push_back({1,
                                                 2,
                                                 DataType::BYTE,
                                                 stride,
                                                 offsetof(TileObjectPrimitive, ctrl),
                                                 VertexInputRate::INSTANCE});
                attribute_descriptions.push_back({1,
                                                 1,
                                                 DataType::INT,
                                                 stride,
                                                 offsetof(TileObjectPrimitive, path_id),
                                                 VertexInputRate::INSTANCE});
                attribute_descriptions.push_back({1,
                                                 1,
                                                 DataType::UNSIGNED_INT,
                                                 stride,
                                                 offsetof(TileObjectPrimitive, metadata_id),
                                                 VertexInputRate::INSTANCE});
            }

            ColorBlendState blend_state = {true, BlendFactor::ONE, BlendFactor::ONE_MINUS_SRC_ALPHA};

            // Create uniform buffers.
            tile_transform_ub = driver->create_buffer(BufferType::Uniform, 16 * sizeof(float));
            tile_varying_sizes_ub = driver->create_buffer(BufferType::Uniform, 8 * sizeof(float));

            // Set descriptor set.
            {
                tile_descriptor_set = std::make_shared<DescriptorSet>();

                // Uniform buffers.

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::Vertex,
                    descriptor.binding = 0;
                    descriptor.binding_name = "bTransform";
                    descriptor.buffer = tile_transform_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::VertexFragment,
                    descriptor.binding = 1;
                    descriptor.binding_name = "bVaryingSizes";
                    descriptor.buffer = tile_varying_sizes_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::VertexFragment,
                    descriptor.binding = 2;
                    descriptor.binding_name = "bFixedSizes";
                    descriptor.buffer = fixed_sizes_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                // Textures.

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.stage = ShaderType::Vertex,
                    descriptor.binding = 1;
                    descriptor.binding_name = "uTextureMetadata";
                    descriptor.texture = metadata_texture;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.stage = ShaderType::Fragment,
                    descriptor.binding = 3;
                    descriptor.binding_name = "uMaskTexture0";
                    descriptor.texture = mask_framebuffer->get_texture();

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }
            }

            tile_pipeline = driver->create_render_pipeline(tile_vert_source,
                                                           tile_frag_source,
                                                           attribute_descriptions,
                                                           blend_state,
                                                           tile_descriptor_set,
                                                           dest_render_pass);
        }
    }

    void RendererD3D9::draw(const SceneBuilderD3D9 &scene_builder) {
        if (scene_builder.pending_fills.empty()) return;

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

    void RendererD3D9::upload_fills(const std::vector<Fill> &fills) {
        auto byte_size = sizeof(Fill) * fills.size();

        if (fill_vertex_buffer == nullptr || byte_size > fill_vertex_buffer->size) {
            fill_vertex_buffer = driver->create_buffer(BufferType::Vertex, byte_size);
        }

        auto cmd_buffer = driver->create_command_buffer();
        cmd_buffer->upload_to_buffer(fill_vertex_buffer,
                                     0,
                                     byte_size,
                                     (void *) fills.data());
        cmd_buffer->submit(driver);
    }

    void RendererD3D9::upload_tiles(const std::vector<TileObjectPrimitive> &tiles) {
        auto byte_size = sizeof(TileObjectPrimitive) * tiles.size();

        if (tile_vertex_buffer == nullptr || byte_size > tile_vertex_buffer->size) {
            tile_vertex_buffer = driver->create_buffer(BufferType::Vertex, byte_size);
        }

        auto cmd_buffer = driver->create_command_buffer();
        cmd_buffer->upload_to_buffer(tile_vertex_buffer,
                                     0,
                                     byte_size,
                                     (void *) tiles.data());
        cmd_buffer->submit(driver);
    }

    void RendererD3D9::upload_and_draw_tiles(const std::vector<DrawTileBatch> &tile_batches,
                                             const std::vector<TextureMetadataEntry> &metadata) {
        // Upload metadata (color, blur, etc...).
        upload_metadata(driver, metadata_texture, metadata);

        need_to_clear_dest = true;

        for (const auto &batch: tile_batches) {
            auto z_buffer_texture = upload_z_buffer(driver, batch.z_buffer_data);

            upload_tiles(batch.tiles);

            draw_tiles(batch.tiles.size(),
                       batch.render_target,
                       batch.color_target,
                       z_buffer_texture);
        }
    }

    void RendererD3D9::draw_fills(uint32_t fills_count) {
        // No fills to draw or no valid mask viewport.
        if (fills_count == 0 || mask_framebuffer == nullptr) return;

        auto cmd_buffer = driver->create_command_buffer();

        cmd_buffer->begin_render_pass(mask_framebuffer,
                                     true,
                                     ColorF());

        cmd_buffer->bind_render_pipeline(fill_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, fill_vertex_buffer});

        cmd_buffer->bind_descriptor_set(fill_descriptor_set);

        cmd_buffer->draw_instanced(6, fills_count);

        cmd_buffer->end_render_pass();

        cmd_buffer->submit(driver);
    }

    void RendererD3D9::draw_tiles(uint32_t tiles_count,
                                  const RenderTarget &render_target,
                                  const RenderTarget &color_target,
                                  const std::shared_ptr<Texture> &z_buffer_texture) {
        // No tiles to draw.
        if (tiles_count == 0) return;

        auto cmd_buffer = driver->create_command_buffer();

        Vec2<float> render_target_size;

        // If no specific RenderTarget is given.
        if (render_target.framebuffer == nullptr) {
            cmd_buffer->begin_render_pass(dest_framebuffer,
                                         need_to_clear_dest,
                                         ColorF());
            render_target_size = {(float) dest_framebuffer->get_width(), (float) dest_framebuffer->get_height()};
            need_to_clear_dest = false;
        } else { // Otherwise, we need to render to that render target.
            cmd_buffer->begin_render_pass(render_target.framebuffer,
                                         true,
                                         ColorF());
            render_target_size = {(float) render_target.framebuffer->get_width(), (float) render_target.framebuffer->get_height()};
        }

        // Update uniform buffers.
        {
            // MVP.
            auto model_mat = Mat4x4<float>(1.f);
            model_mat = model_mat.translate(Vec3<float>(-1.f, 1.f, 0.f));
            model_mat = model_mat.scale(Vec3<float>(2.f / render_target_size.x, -2.f / render_target_size.y, 1.f));
            auto mvp_mat = model_mat;

            std::array<float, 6> ubo_data = {(float) z_buffer_texture->get_width(), (float) z_buffer_texture->get_height(),
                                             (float) color_target.size.x, (float) color_target.size.y,
                                             render_target_size.x, render_target_size.y};

            auto one_shot_cmd_buffer = driver->create_command_buffer();
            one_shot_cmd_buffer->upload_to_buffer(tile_transform_ub, 0, 16 * sizeof(float), &mvp_mat);
            one_shot_cmd_buffer->upload_to_buffer(tile_varying_sizes_ub, 0, 6 * sizeof(float), ubo_data.data());
            one_shot_cmd_buffer->submit(driver);
        }

        // Update descriptor set.
        {
            tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, ShaderType::Fragment, 0, "uDestTexture", nullptr, nullptr});
            tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, ShaderType::Vertex, 2, "uZBuffer", nullptr, z_buffer_texture});

            if (color_target.framebuffer != nullptr) {
                tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, ShaderType::Fragment, 4, "uColorTexture0", nullptr, color_target.framebuffer->get_texture()});
            }
        }

        cmd_buffer->bind_render_pipeline(tile_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, tile_vertex_buffer});

        cmd_buffer->bind_descriptor_set(tile_descriptor_set);

        cmd_buffer->draw_instanced(6, tiles_count);

        cmd_buffer->end_render_pass();

        cmd_buffer->submit(driver);
    }

    std::shared_ptr<Texture> RendererD3D9::get_dest_texture() {
        return dest_framebuffer->get_texture();
    }
}
