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
                                             const DenseTileMap<uint32_t> &z_buffer_map,
                                             const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        auto z_buffer_texture = driver->create_texture(
                z_buffer_map.rect.width(),
                z_buffer_map.rect.height(),
                TextureFormat::RGBA8_UNORM);

        cmd_buffer->upload_to_texture(z_buffer_texture, {}, z_buffer_map.data.data());

        return z_buffer_texture;
    }

    RendererD3D9::RendererD3D9(const std::shared_ptr<Driver> &p_driver, uint32_t canvas_width, uint32_t canvas_height)
            : Renderer(p_driver) {
        mask_render_pass = driver->create_render_pass(TextureFormat::RGBA16F, AttachmentLoadOp::CLEAR,
                                                      ImageLayout::SHADER_READ_ONLY);

        dest_render_pass_clear = driver->create_render_pass(TextureFormat::RGBA8_UNORM, AttachmentLoadOp::CLEAR,
                                                            ImageLayout::SHADER_READ_ONLY);
        dest_render_pass_load = driver->create_render_pass(TextureFormat::RGBA8_UNORM, AttachmentLoadOp::LOAD,
                                                           ImageLayout::SHADER_READ_ONLY);

        mask_framebuffer = driver->create_framebuffer(MASK_FRAMEBUFFER_WIDTH,
                                                      MASK_FRAMEBUFFER_HEIGHT,
                                                      TextureFormat::RGBA16F,
                                                      mask_render_pass);

        dest_framebuffer = driver->create_framebuffer(canvas_width,
                                                      canvas_height,
                                                      TextureFormat::RGBA8_UNORM,
                                                      dest_render_pass_clear);

        // Quad vertex buffer. Shared by fills and tiles drawing.
        quad_vertex_buffer = driver->create_buffer(BufferType::Vertex, 12 * sizeof(uint16_t),
                                                   MemoryProperty::DEVICE_LOCAL);

        auto cmd_buffer = driver->create_command_buffer(true);
        cmd_buffer->upload_to_buffer(quad_vertex_buffer, 0, 12 * sizeof(uint16_t), QUAD_VERTEX_POSITIONS);
        cmd_buffer->submit(driver);
    }

    void RendererD3D9::set_up_pipelines(uint32_t canvas_width, uint32_t canvas_height) {
        // Fill pipeline.
        {
#ifdef PATHFINDER_USE_VULKAN
            const auto fill_vert_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/fill_vert.spv");
            const auto fill_frag_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/fill_frag.spv");
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
                fill_descriptor_set = driver->create_descriptor_set();

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
                                                                      1,
                                                                      "uAreaLUT",
                                                                      nullptr,
                                                                      area_lut_texture});
            }

            fill_pipeline = driver->create_render_pipeline(fill_vert_source,
                                                           fill_frag_source,
                                                           attribute_descriptions,
                                                           blend_state,
                                                           {MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT},
                                                           fill_descriptor_set,
                                                           mask_render_pass);
        }

        // Tile pipeline.
        {
#ifdef PATHFINDER_USE_VULKAN
            const auto tile_vert_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/tile_vert.spv");
            const auto tile_frag_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d9/tile_frag.spv");
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
                                                  2 * sizeof(uint16_t),
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
            tile_transform_ub = driver->create_buffer(BufferType::Uniform, 16 * sizeof(float),
                                                      MemoryProperty::HOST_VISIBLE_AND_COHERENT);
            tile_varying_sizes_ub = driver->create_buffer(BufferType::Uniform, 8 * sizeof(float),
                                                          MemoryProperty::HOST_VISIBLE_AND_COHERENT);

            // Set descriptor set.
            {
                tile_descriptor_set = driver->create_descriptor_set();

                // Uniform buffers.

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::Vertex,
                            descriptor.binding = 2;
                    descriptor.binding_name = "bTransform";
                    descriptor.buffer = tile_transform_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::VertexFragment,
                            descriptor.binding = 3;
                    descriptor.binding_name = "bVaryingSizes";
                    descriptor.buffer = tile_varying_sizes_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.stage = ShaderType::VertexFragment,
                            descriptor.binding = 4;
                    descriptor.binding_name = "bFixedSizes";
                    descriptor.buffer = fixed_sizes_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                // Textures.

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.stage = ShaderType::Vertex,
                            descriptor.binding = 0;
                    descriptor.binding_name = "uTextureMetadata";
                    descriptor.texture = metadata_texture;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.stage = ShaderType::Fragment,
                            descriptor.binding = 6;
                    descriptor.binding_name = "uMaskTexture0";
                    descriptor.texture = mask_framebuffer->get_texture();

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                // Placeholders.
                tile_descriptor_set->add_or_update_descriptor(
                        {DescriptorType::Texture, ShaderType::Fragment, 7, "uDestTexture", nullptr, nullptr});
                tile_descriptor_set->add_or_update_descriptor(
                        {DescriptorType::Texture, ShaderType::Fragment, 8, "uGammaLUT", nullptr, nullptr});
                tile_descriptor_set->add_or_update_descriptor(
                        {DescriptorType::Texture, ShaderType::Vertex, 1, "uZBuffer", nullptr, nullptr});
                tile_descriptor_set->add_or_update_descriptor(
                        {DescriptorType::Texture, ShaderType::Fragment, 5, "uColorTexture0", nullptr, nullptr});
            }

            tile_pipeline = driver->create_render_pipeline(tile_vert_source,
                                                           tile_frag_source,
                                                           attribute_descriptions,
                                                           blend_state,
                                                           {canvas_width, canvas_height},
                                                           tile_descriptor_set,
                                                           dest_render_pass_clear);
        }
    }

    void RendererD3D9::draw(const SceneBuilderD3D9 &scene_builder) {
        if (scene_builder.pending_fills.empty()) return;

        auto cmd_buffer = driver->create_command_buffer(true);

        // We are supposed to do this before the builder finishes building.
        // But it doesn't improve much performance, so we just leave it as it is for the sake of simplicity.
        {
            // Upload fills to buffer.
            upload_fills(scene_builder.pending_fills, cmd_buffer);

            // We can do fill drawing as soon as the fill vertex buffer is ready.
            draw_fills(scene_builder.pending_fills.size(), cmd_buffer);
        }

        // Tiles need to be drawn after fill drawing and after tile batches are prepared.
        upload_and_draw_tiles(scene_builder.tile_batches, scene_builder.metadata, cmd_buffer);

        cmd_buffer->submit(driver);
    }

    void RendererD3D9::upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        auto byte_size = sizeof(Fill) * fills.size();

        // If we need to allocate a new fill vertex buffer.
        if (fill_vertex_buffer == nullptr || byte_size > fill_vertex_buffer->size) {
            fill_vertex_buffer = driver->create_buffer(BufferType::Vertex,
                                                       byte_size,
                                                       MemoryProperty::DEVICE_LOCAL);
        }

        cmd_buffer->upload_to_buffer(fill_vertex_buffer,
                                     0,
                                     byte_size,
                                     (void *) fills.data());
    }

    void RendererD3D9::upload_tiles(const std::vector<TileObjectPrimitive> &tiles,
                                    const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        auto byte_size = sizeof(TileObjectPrimitive) * tiles.size();

        // If we need to allocate a new tile vertex buffer.
        if (tile_vertex_buffer == nullptr || byte_size > tile_vertex_buffer->size) {
            tile_vertex_buffer = driver->create_buffer(BufferType::Vertex,
                                                       byte_size,
                                                       MemoryProperty::DEVICE_LOCAL);
        }

        cmd_buffer->upload_to_buffer(tile_vertex_buffer,
                                     0,
                                     byte_size,
                                     (void *) tiles.data());
    }

    void RendererD3D9::upload_and_draw_tiles(const std::vector<DrawTileBatch> &tile_batches,
                                             const std::vector<TextureMetadataEntry> &metadata,
                                             const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        // Upload metadata (color, blur, etc...).
        upload_metadata(metadata_texture, metadata, cmd_buffer);

        // Clear the destination framebuffer for the first time.
        need_to_clear_dest = true;

        for (const auto &batch: tile_batches) {
            uint32_t tile_count = batch.tiles.size();

            // No tiles to draw.
            if (tile_count == 0) continue;

            auto z_buffer_texture = upload_z_buffer(driver, batch.z_buffer_data, cmd_buffer);

            upload_tiles(batch.tiles, cmd_buffer);

            draw_tiles(tile_count,
                       batch.render_target,
                       batch.color_texture,
                       z_buffer_texture,
                       cmd_buffer);
        }
    }

    void RendererD3D9::draw_fills(uint32_t fills_count, const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        // No fills to draw or no valid mask viewport.
        if (fills_count == 0 || mask_framebuffer == nullptr) return;

        cmd_buffer->begin_render_pass(mask_render_pass,
                                      mask_framebuffer,
                                      ColorF());

        cmd_buffer->bind_render_pipeline(fill_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, fill_vertex_buffer});

        cmd_buffer->bind_descriptor_set(fill_descriptor_set);

        cmd_buffer->draw_instanced(6, fills_count);

        cmd_buffer->end_render_pass();
    }

    void RendererD3D9::draw_tiles(uint32_t tiles_count,
                                  const RenderTarget &render_target,
                                  const std::shared_ptr<Texture> &color_texture,
                                  const std::shared_ptr<Texture> &z_buffer_texture,
                                  const std::shared_ptr<CommandBuffer> &cmd_buffer) {
        std::shared_ptr<Framebuffer> target_framebuffer;

        // If no specific RenderTarget is given.
        if (render_target.framebuffer == nullptr) {
            cmd_buffer->begin_render_pass(need_to_clear_dest ? dest_render_pass_clear : dest_render_pass_load,
                                          dest_framebuffer,
                                          ColorF());

            target_framebuffer = dest_framebuffer;

            need_to_clear_dest = false;
        } else { // Otherwise, we need to render to that render target.
            cmd_buffer->begin_render_pass(dest_render_pass_clear,
                                          render_target.framebuffer,
                                          ColorF());

            target_framebuffer = render_target.framebuffer;
        }

        Vec2<float> target_framebuffer_size = {(float) target_framebuffer->get_width(),
                                               (float) target_framebuffer->get_height()};

        // Update uniform buffers.
        {
            // MVP (with only the model matrix).
            auto model_mat = Mat4x4<float>(1.f);
            model_mat = model_mat.translate(Vec3<float>(-1.f, -1.f, 0.f)); // Move to top-left.
            model_mat = model_mat.scale(Vec3<float>(2.f / target_framebuffer_size.x, 2.f / target_framebuffer_size.y, 1.f));

            std::array<float, 6> ubo_data = {
                    (float) z_buffer_texture->get_width(),
                    (float) z_buffer_texture->get_height(),
                    color_texture ? (float) color_texture->get_width() : 0,
                    color_texture ? (float) color_texture->get_width() : 0,
                    target_framebuffer_size.x,
                    target_framebuffer_size.y
            };

            // We don't need to preserve the data until the upload commands are implemented because
            // these uniform buffers are host-visible/coherent.
            cmd_buffer->upload_to_buffer(tile_transform_ub, 0, 16 * sizeof(float), &model_mat);
            cmd_buffer->upload_to_buffer(tile_varying_sizes_ub, 0, 6 * sizeof(float), ubo_data.data());
        }

        // Update descriptor set.
        {
            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::Texture, ShaderType::Fragment, 7, "uDestTexture", nullptr,
                     z_buffer_texture}); // Unused
            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::Texture, ShaderType::Vertex, 1, "uZBuffer", nullptr, z_buffer_texture});
            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::Texture, ShaderType::Fragment, 5, "uColorTexture0", nullptr,
                     color_texture ? color_texture : z_buffer_texture});
            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::Texture, ShaderType::Fragment, 8, "uGammaLUT", nullptr,
                     z_buffer_texture}); // Unused
        }

        cmd_buffer->bind_render_pipeline(tile_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, tile_vertex_buffer});

        cmd_buffer->bind_descriptor_set(tile_descriptor_set);

        cmd_buffer->draw_instanced(6, tiles_count);

        cmd_buffer->end_render_pass();
    }

    std::shared_ptr<Texture> RendererD3D9::get_dest_texture() {
        return dest_framebuffer->get_texture();
    }
}
