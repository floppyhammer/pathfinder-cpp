#include "scene_builder.h"

#include <thread>

#include "../../common/global_macros.h"
#include "../../common/timestamp.h"
#include "../scene.h"
#include "renderer.h"
#include "tiler.h"

#undef min
#undef max

namespace Pathfinder {

/// Create tile batches. Different batches use different color textures.
std::vector<DrawTileBatchD3D9> build_tile_batches_for_draw_path_display_item(
    const Scene &scene,
    const std::vector<BuiltDrawPath> &built_paths,
    Range draw_path_range) {
    std::vector<DrawTileBatchD3D9> flushed_draw_tile_batches;

    // New draw tile batch.
    std::shared_ptr<DrawTileBatchD3D9> draw_tile_batch;

    for (auto draw_path_id = draw_path_range.start; draw_path_id < draw_path_range.end; draw_path_id++) {
        const auto &draw_path = built_paths[draw_path_id];
        const auto &path_data = draw_path.path.data;

        // If we should create a new batch.
        bool flush_needed = false;

        // Try to reuse the current batch if we can.
        if (draw_tile_batch) {
            flush_needed = !fixup_batch_for_new_path_if_possible(draw_tile_batch->color_texture_info, draw_path);
        }

        // If we couldn't reuse the batch, flush it.
        if (flush_needed) {
            flushed_draw_tile_batches.push_back(*draw_tile_batch);
            draw_tile_batch = nullptr;
        }

        if (draw_tile_batch == nullptr) {
            draw_tile_batch = std::make_shared<DrawTileBatchD3D9>();

            // TODO(floppyhammer): Currently, Z buffers of different batches have got
            //  the same size (the scene's view box).
            //  Maybe we should have Z buffer's size depend on its batch?
            auto tile_bounds = round_rect_out_to_tile_bounds(scene.get_view_box());

            draw_tile_batch->z_buffer_data = DenseTileMap<uint32_t>::z_builder(tile_bounds);
            draw_tile_batch->color_texture_info = draw_path.color_texture_info;
        }

        for (const auto &tile : path_data.tiles.data) {
            // If not an alpha tile and winding is zero.
            if (!tile.alpha_tile_id.is_valid() && tile.backdrop == 0) {
                continue;
            }

            draw_tile_batch->tiles.push_back(tile);

            // Z buffer is only meant for visible SOLID tiles and not for any ALPHA tiles.
            if (!draw_path.occludes || tile.alpha_tile_id.is_valid()) {
                continue;
            }

            // Set z buffer value.
            // ----------------------------------------------------------
            // Get tile index in the vector.
            auto z_buffer_index = draw_tile_batch->z_buffer_data.coords_to_index_unchecked({tile.tile_x, tile.tile_y});
            auto z_value = &draw_tile_batch->z_buffer_data.data[z_buffer_index];

            // Store the biggest draw_path_id as the z value, which means the solid tile of this path is the topmost.
            *z_value = std::max(*z_value, (unsigned int)draw_path_id);
            // ----------------------------------------------------------
        }

        if (path_data.clip_tiles) {
            for (auto &clip_tile : path_data.clip_tiles->data) {
                if (clip_tile.dest_tile_id.is_valid() && clip_tile.src_tile_id.is_valid()) {
                    draw_tile_batch->clips.push_back(clip_tile);
                }
            }
        }
    }

    if (draw_tile_batch) {
        flushed_draw_tile_batches.push_back(*draw_tile_batch);
    }

    return flushed_draw_tile_batches;
}

void SceneBuilderD3D9::build(Scene *_scene, Renderer *renderer) {
    scene = _scene;

    // Build paint data.
    auto paint_metadata = scene->palette.build_paint_info(renderer);

    // Most important step.
    // Build draw paths into built draw paths.
    auto built_paths = build_paths_on_cpu(paint_metadata);

    // Prepare batches.
    finish_building(built_paths);
}

void SceneBuilderD3D9::finish_building(const std::vector<BuiltDrawPath> &built_paths) {
    // We can already start drawing fills asynchronously at this stage.
    build_tile_batches(built_paths);
}

std::vector<BuiltDrawPath> SceneBuilderD3D9::build_paths_on_cpu(std::vector<PaintMetadata> &paint_metadata) {
    // Reset builder.
    // ------------------------------
    // Clear pending fills.
    pending_fills.clear();

    // Clear next alpha tile indices.
    for (auto &next_alpha_tile_index : next_alpha_tile_indices) {
        next_alpha_tile_index = 0;
    }
    // ------------------------------

    auto draw_paths_count = scene->draw_paths.size();
    auto clip_paths_count = scene->clip_paths.size();
    auto view_box = scene->get_view_box();

#ifdef __EMSCRIPTEN__
    std::vector<BuiltPath> built_clip_paths(clip_paths_count);

    for (uint32_t path_index = 0; path_index < clip_paths_count; path_index++) {
        built_clip_paths[path_index] = build_clip_path_on_cpu(PathBuildParams{path_index, view_box, scene});
    }

    std::vector<BuiltDrawPath> built_draw_paths(draw_paths_count);

    for (uint32_t path_index = 0; path_index < draw_paths_count; path_index++) {
        auto params = DrawPathBuildParams({path_index, view_box, scene}, paint_metadata, built_clip_paths);

        built_draw_paths[path_index] = build_draw_path_on_cpu(params);
    }
#else
    // We need to build clip paths first.
    std::vector<BuiltPath> built_clip_paths(clip_paths_count);
    {
        // Parallel build.
        auto task = [this, &built_clip_paths, &clip_paths_count, &view_box](int begin) {
            for (uint32_t path_index = begin; path_index < clip_paths_count; path_index += PATHFINDER_THREADS) {
                built_clip_paths[path_index] = build_clip_path_on_cpu(PathBuildParams{path_index, view_box, scene});
            }
        };

        size_t threads_count = std::min(clip_paths_count, (size_t)PATHFINDER_THREADS);

        std::vector<std::thread> threads(threads_count);

        for (int i = 0; i < threads_count; i++) {
            threads[i] = std::thread(task, i);
        }

        for (auto &t : threads) {
            t.join();
        }
    }

    std::vector<BuiltDrawPath> built_draw_paths(draw_paths_count);
    {
        // Parallel build.
        auto task = [this, &built_draw_paths, &draw_paths_count, &view_box, &paint_metadata, &built_clip_paths](
                        int begin) {
            for (uint32_t path_index = begin; path_index < draw_paths_count; path_index += PATHFINDER_THREADS) {
                auto params =
                    DrawPathBuildParams(PathBuildParams{path_index, view_box, scene}, paint_metadata, built_clip_paths);

                built_draw_paths[path_index] = build_draw_path_on_cpu(params);
            }
        };

        size_t threads_count = std::min(draw_paths_count, (size_t)PATHFINDER_THREADS);

        std::vector<std::thread> threads(threads_count);

        for (int i = 0; i < threads_count; i++) {
            threads[i] = std::thread(task, i);
        }

        for (auto &t : threads) {
            t.join();
        }
    }
#endif

    return built_draw_paths;
}

BuiltPath SceneBuilderD3D9::build_clip_path_on_cpu(const PathBuildParams &params) {
    uint32_t path_id = params.path_id;

    const auto &path_object = scene->clip_paths[path_id];

    TilingPathInfo tiling_path_info;
    tiling_path_info.type = TilingPathInfo::Type::Clip;

    // Create a tiler for the clip path.
    Tiler tiler(*this,
                path_id,
                path_object.outline,
                path_object.fill_rule,
                scene->get_view_box(),
                path_object.clip_path,
                {},
                tiling_path_info);

    // Core step.
    tiler.generate_tiles();

    // Add generated fills from the tile generation step.
    send_fills(tiler.object_builder.fills);

    return tiler.object_builder.built_path;
}

BuiltDrawPath SceneBuilderD3D9::build_draw_path_on_cpu(const DrawPathBuildParams &params) {
    uint32_t path_id = params.path_build_params.path_id;

    const auto &path_object = scene->draw_paths[path_id];

    TilingPathInfo path_info{};
    path_info.type = TilingPathInfo::Type::Draw;
    path_info.info.paint_id = path_object.paint;
    path_info.info.blend_mode = path_object.blend_mode;
    path_info.info.fill_rule = path_object.fill_rule;

    auto paint_id = path_object.paint;
    auto &_paint_metadata = params.paint_metadata[paint_id];

    // Create a tiler for the draw path.
    Tiler tiler(*this,
                path_id,
                path_object.outline,
                path_object.fill_rule,
                params.path_build_params.view_box,
                path_object.clip_path,
                params.built_clip_paths,
                path_info);

    // Core step.
    tiler.generate_tiles();

    // Send the fills generated from the tile generation step.
    send_fills(tiler.object_builder.fills);

    return {tiler.object_builder.built_path, path_object, _paint_metadata};
}

void SceneBuilderD3D9::build_tile_batches(const std::vector<BuiltDrawPath> &built_paths) {
    // Clear batches.
    tile_batches.clear();

    std::vector<RenderTargetId> render_target_stack;

    // Build batches using the display items.
    for (const auto &display_item : scene->display_list) {
        switch (display_item.type) {
            case DisplayItem::Type::PushRenderTarget: {
                render_target_stack.push_back(display_item.render_target_id);
            } break;
            case DisplayItem::Type::PopRenderTarget: {
                render_target_stack.pop_back();
            } break;
            case DisplayItem::Type::DrawPaths: {
                // Create new batches.
                auto batches = build_tile_batches_for_draw_path_display_item(*scene, built_paths, display_item.range);

                for (auto &batch : batches) {
                    // Set render target of the batches.
                    if (!render_target_stack.empty()) {
                        // Fetch the render target on the top of the stack.
                        batch.render_target_id = std::make_shared<RenderTargetId>(render_target_stack.back());
                    }

                    tile_batches.push_back(batch);
                }
            } break;
        }
    }
}

void SceneBuilderD3D9::send_fills(const std::vector<Fill> &fill_batch) {
    // Need a lock.
    fill_write_mutex.lock();
    pending_fills.insert(pending_fills.end(), fill_batch.begin(), fill_batch.end());
    fill_write_mutex.unlock();
}

} // namespace Pathfinder
