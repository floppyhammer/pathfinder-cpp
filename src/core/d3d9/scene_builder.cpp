#include "scene_builder.h"
#include "tiler.h"
#include "../../common/timestamp.h"
#include "../../common/global_macros.h"

#undef min
#undef max

namespace Pathfinder {
    /// Create tile batches.
    DrawTileBatch build_tile_batches_for_draw_shape_display_item(Scene &p_scene,
                                                                const std::vector<BuiltDrawShape> &built_shapes,
                                                                Range draw_shape_range) {
        // New draw tile batch.
        DrawTileBatch draw_tile_batch;

        auto tile_bounds = round_rect_out_to_tile_bounds(p_scene.view_box);

        draw_tile_batch.z_buffer_data = DenseTileMap<uint32_t>::z_builder(tile_bounds);

        for (unsigned long draw_shape_id = draw_shape_range.start; draw_shape_id < draw_shape_range.end; draw_shape_id++) {
            const auto &draw_shape = built_shapes[draw_shape_id];
            const auto &cpu_data = draw_shape.shape.data;

            for (const auto &tile: cpu_data.tiles.data) {
                // If not an alpha tile and winding is zero.
                if (!tile.alpha_tile_id.is_valid() && tile.backdrop == 0) {
                    continue;
                }

                draw_tile_batch.tiles.push_back(tile);

                // Z buffer is only meant for visible SOLID tiles and not for any ALPHA tiles.
                if (!draw_shape.occludes || tile.alpha_tile_id.is_valid()) {
                    continue;
                }

                // Set z buffer value.
                // ----------------------------------------------------------
                // Get tile index in the vector.
                auto z_buffer_index = draw_tile_batch.z_buffer_data.coords_to_index_unchecked(tile.tile_x, tile.tile_y);
                auto z_value = &draw_tile_batch.z_buffer_data.data[z_buffer_index];

                // Store the biggest draw_shape_id as the z value, which means the solid tile of this path is the topmost.
                *z_value = std::max(*z_value, (unsigned int) draw_shape_id);
                // ----------------------------------------------------------
            }
        }

        return draw_tile_batch;
    }

    void SceneBuilderD3D9::build() {
        // No need to rebuild the scene if it hasn't changed.
        // Comment this to do benchmark more precisely.
//        if (!scene->is_dirty)
//            return;

        Timestamp timestamp;

        // Build paint data.
        metadata = scene->palette.build_paint_info();

        timestamp.record("Build paint info");

        // Most important step. Build shapes into built draw shapes.
        auto built_shapes = build_shapes_on_cpu();

        timestamp.record("Build paths on CPU");

        finish_building(built_shapes);

        timestamp.record("Build tile batches");
        timestamp.print();

        // Mark the scene as clean, so we don't need to rebuild it the next frame.
        scene->is_dirty = false;
    }

    void SceneBuilderD3D9::finish_building(const std::vector<BuiltDrawShape> &built_shapes) {
        // We can already start drawing fills asynchronously at this stage.

        build_tile_batches(built_shapes);
    }

    std::vector<BuiltDrawShape> SceneBuilderD3D9::build_shapes_on_cpu() {
        // Reset builder.
        {
            // Clear pending fills.
            pending_fills.clear();

            // Reset next alpha tile indices.
            std::fill(next_alpha_tile_indices, next_alpha_tile_indices + ALPHA_TILE_LEVEL_COUNT, 0);
        }

        // Number of draw shapes.
        auto draw_shapes_count = scene->draw_shapes.size();

        // Allocate space for built draw shapes.
        std::vector<BuiltDrawShape> built_shapes(draw_shapes_count);

        // Set up an OpenMP lock.
        omp_lock_t write_lock;
        omp_init_lock(&write_lock);

        // This loop should run in parallel whenever possible.
#if (PATHFINDER_OPENMP_THREADS > 1)
#pragma omp parallel for num_threads(PATHFINDER_OPENMP_THREADS)
#endif
        for (int path_index = 0; path_index < draw_shapes_count; path_index++) {
            // Retrieve a draw shape.
            auto &draw_shape = scene->draw_shapes[path_index];

            // Skip if it's invisible (transparent or out of scope).
            if (!scene->get_paint(draw_shape.paint).is_opaque()
                || !draw_shape.bounds.intersects(scene->view_box)) {
                continue;
            }

            // Build the draw shape.
            built_shapes[path_index] = build_draw_shape_on_cpu(path_index, write_lock);
        }

        omp_destroy_lock(&write_lock);

        return built_shapes;
    }

    BuiltDrawShape SceneBuilderD3D9::build_draw_shape_on_cpu(uint32_t shape_id, omp_lock_t &write_lock) {
        // Get the draw shape.
        const auto &path_object = scene->draw_shapes[shape_id];

        // Create a tiler for the draw shape.
        Tiler tiler(*this, shape_id, path_object, path_object.fill_rule, scene->view_box);

        // Core step.
        tiler.generate_tiles();

        tiler.object_builder.built_shape.paint_id = path_object.paint;

        // Add generated fills from the tile generation step. Need a lock.
        omp_set_lock(&write_lock);
        send_fills(tiler.object_builder.fills);
        omp_unset_lock(&write_lock);

        return {tiler.object_builder.built_shape, path_object.blend_mode, path_object.fill_rule, true};
    }

    void SceneBuilderD3D9::build_tile_batches(const std::vector<BuiltDrawShape> &built_shapes) {
        // Clear batches.
        tile_batches.clear();

        std::vector<RenderTarget> render_target_stack;

        // Build batches using the display items.
        for (const auto &display_item: scene->display_list) {
            switch (display_item.type) {
                case DisplayItem::Type::PushRenderTarget: {
                    render_target_stack.push_back(display_item.render_target);
                }
                    break;
                case DisplayItem::Type::PopRenderTarget: {
                    render_target_stack.pop_back();
                }
                    break;
                case DisplayItem::Type::DrawPaths: {
                    // Create a new batch.
                    auto tile_batch = build_tile_batches_for_draw_shape_display_item(
                            *scene,
                            built_shapes,
                            display_item.path_range);

                    // Get paint.
                    auto paint_id = built_shapes[display_item.path_range.start].shape.paint_id;
                    Paint paint = scene->palette.get_paint(paint_id);

                    // Set color texture.
                    auto overlay = paint.get_overlay();
                    if (overlay && overlay->contents.pattern) {
                        auto pattern = overlay->contents.pattern;
                        if (pattern->source.type == PatternSource::Type::Image) {
                            // FIXME: Make it work.
                            tile_batch.color_texture = nullptr;
                        } else {
                            tile_batch.color_texture = overlay->contents.pattern->source.render_target.framebuffer->get_texture();
                        }
                    }

                    // Set render target. Pick the one on the top of the stack.
                    if (!render_target_stack.empty()) {
                        tile_batch.render_target = render_target_stack.back();
                    }

                    tile_batches.push_back(tile_batch);
                }
                    break;
            }
        }
    }

    void SceneBuilderD3D9::send_fills(const std::vector<Fill> &fill_batch) {
        pending_fills.insert(pending_fills.end(),
                             fill_batch.begin(),
                             fill_batch.end());
    }
}
