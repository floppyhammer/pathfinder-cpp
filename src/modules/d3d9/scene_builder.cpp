//
// Created by chy on 6/24/2021.
//

#include "scene_builder.h"
#include "tiler.h"
#include "../../common/timestamp.h"
#include "../../common/global_macros.h"

#include <omp.h>

#undef min
#undef max

namespace Pathfinder {
    /// Create tile batches.
    DrawTileBatch build_tile_batches_for_draw_path_display_item(
            Scene &p_scene,
            const std::vector<BuiltDrawPath> &built_paths,
            Range draw_path_range) {
        // New draw tile batch.
        DrawTileBatch draw_tile_batch;

        auto tile_bounds = round_rect_out_to_tile_bounds(p_scene.view_box);

        draw_tile_batch.z_buffer_data = DenseTileMap<uint32_t>::z_builder(tile_bounds);

        for (unsigned long draw_path_id = draw_path_range.start; draw_path_id < draw_path_range.end; draw_path_id++) {
            const auto &draw_path = built_paths[draw_path_id];
            const auto &cpu_data = built_paths[draw_path_id].path.data;

            for (const auto &tile: cpu_data.tiles.data) {
                // If not an alpha tile and winding is zero.
                if (!tile.alpha_tile_id.is_valid() && tile.backdrop == 0) {
                    continue;
                }

                draw_tile_batch.tiles.push_back(tile);

                // Z buffer is only meant for visible SOLID tiles and not for any ALPHA tiles.
                if (!draw_path.occludes || tile.alpha_tile_id.is_valid()) {
                    continue;
                }

                // Set z buffer value.
                // ----------------------------------------------------------
                // Get tile index in the vector.
                auto z_buffer_index = draw_tile_batch.z_buffer_data.coords_to_index_unchecked(tile.tile_x, tile.tile_y);
                auto z_value = &draw_tile_batch.z_buffer_data.data[z_buffer_index];

                // Store the biggest draw_path_id as the z value, which means the solid tile of this path is the topmost.
                *z_value = std::max(*z_value, (unsigned int) draw_path_id);
                // ----------------------------------------------------------
            }
        }

        return draw_tile_batch;
    }

    void SceneBuilderD3D9::build() {
        // No need to rebuild the scene.
        //if (!scene->is_dirty)
        //    return;

        Timestamp timestamp;

        // Build paint data.
        metadata = scene->palette.build_paint_info();

        timestamp.record("build_paint_info");

        // Most important step. Build paths (i.e. shapes). Draw paths -> built draw paths.
        auto built_paths = build_paths_on_cpu();

        timestamp.record("build_paths_on_cpu");

        finish_building(built_paths);

        timestamp.record("finish_building");

        // Mark the scene as clean (so we don't need to rebuild it the next frame).
        scene->is_dirty = false;

        timestamp.print();
    }

    void SceneBuilderD3D9::finish_building(const std::vector<BuiltDrawPath> &built_paths) {
        build_tile_batches(built_paths);
    }

    std::vector<BuiltDrawPath> SceneBuilderD3D9::build_paths_on_cpu() {
        // Reset builder.
        // ------------------------------
        // Clear pending fills.
        pending_fills.clear();

        // Clear next alpha tile indices.
        for (auto& next_alpha_tile_index : next_alpha_tile_indices)
            next_alpha_tile_index = 0;
        // ------------------------------

        auto draw_paths_count = scene->draw_paths.size();

        std::vector<BuiltDrawPath> built_paths(draw_paths_count);

        omp_lock_t write_lock;
        omp_init_lock(&write_lock);

        // This should run in parallel when possible.
#if (PATHFINDER_OPENMP_THREADS > 1)
#pragma omp parallel for num_threads(PATHFINDER_OPENMP_THREADS)
#endif
        for (int path_index = 0; path_index < draw_paths_count; path_index++) {
            auto& draw_path = scene->draw_paths[path_index];

            // Skip invisible draw paths.
            if (!scene->get_paint(draw_path.paint).is_opaque()
                || !draw_path.bounds.intersects(scene->view_box)) {
                continue;
            }

            built_paths[path_index] = build_draw_path_on_cpu(path_index, write_lock);
        }

        omp_destroy_lock(&write_lock);

        return built_paths;
    }

    BuiltDrawPath SceneBuilderD3D9::build_draw_path_on_cpu(uint32_t path_id, omp_lock_t &write_lock) {
        // Get draw path (thin wrapper of outline). This is just a normal shape.
        auto &path_object = scene->draw_paths[path_id];

        // Create a tiler for the path.
        Tiler tiler(*this, path_id, path_object, path_object.fill_rule, scene->view_box);

        // Core step.
        tiler.generate_tiles();

        tiler.object_builder.built_path.paint_id = path_object.paint;

        // Add generated fills from the tile generation step. Need a lock.
        omp_set_lock(&write_lock);
        send_fills(tiler.object_builder.fills);
        omp_unset_lock(&write_lock);

        return { tiler.object_builder.built_path, path_object.blend_mode, path_object.fill_rule, true };
    }

    void SceneBuilderD3D9::build_tile_batches(const std::vector<BuiltDrawPath> &built_paths) {
        // Clear batches.
        tile_batches.clear();

        std::vector<RenderTarget> render_target_stack;

        // Prepare display items.
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
                    auto tile_batch = build_tile_batches_for_draw_path_display_item(*scene,
                                                                                    built_paths,
                                                                                    display_item.path_range);
                    auto paint_id = built_paths[display_item.path_range.start].path.paint_id;
                    Paint paint = scene->palette.get_paint(paint_id);
                    auto overlay = paint.get_overlay();
                    if (overlay && overlay->contents.pattern) {
                        tile_batch.color_texture = overlay->contents.pattern->source.render_target;
                    }

                    // Set render target. Render to screen if there's no render targets on the stack.
                    if (!render_target_stack.empty()) {
                        tile_batch.viewport = render_target_stack.back();
                    }

                    tile_batches.push_back(tile_batch);
                }
                    break;
            }
        }
    }

    void SceneBuilderD3D9::send_fills(std::vector<Fill> &fill_batch) {
        // Extend.
        pending_fills.insert(pending_fills.end(),
                             fill_batch.begin(),
                             fill_batch.end());
    }
}
