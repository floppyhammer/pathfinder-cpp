#include "scene_builder.h"

#include <thread>

#include "../../common/global_macros.h"
#include "../../common/timestamp.h"
#include "../scene.h"
#include "tiler.h"

#undef min
#undef max

namespace Pathfinder {

/// Create tile batches.
DrawTileBatch build_tile_batches_for_draw_path_display_item(Scene &scene,
                                                            const std::vector<BuiltDrawPath> &built_paths,
                                                            Range draw_path_range) {
    // New draw tile batch.
    DrawTileBatch draw_tile_batch;

    auto tile_bounds = round_rect_out_to_tile_bounds(scene.get_view_box());

    draw_tile_batch.z_buffer_data = DenseTileMap<uint32_t>::z_builder(tile_bounds);
    draw_tile_batch.metadata_texture = scene.palette.metadata_texture;

    for (auto draw_path_id = draw_path_range.start; draw_path_id < draw_path_range.end; draw_path_id++) {
        const auto &draw_path = built_paths[draw_path_id];
        const auto &path_data = draw_path.path.data;

        // For paint overlay.
        {
            // Get paint.
            auto paint_id = draw_path.path.paint_id;
            Paint paint = scene.palette.get_paint(paint_id);

            auto overlay = paint.get_overlay();

            // Set color texture.
            if (overlay) {
                if (overlay->contents.type == PaintContents::Type::Gradient) {
                    auto gradient = overlay->contents.gradient;

                    draw_tile_batch.color_texture = gradient.tile_texture;
                } else {
                    auto pattern = overlay->contents.pattern;

                    // Source is an image.
                    if (pattern.source.type == PatternSource::Type::Image) {
                        draw_tile_batch.color_texture = pattern.source.image.texture;
                    } else { // Source is a render target.
                        draw_tile_batch.color_texture =
                            overlay->contents.pattern.source.render_target.framebuffer->get_texture();
                    }
                }
            }
        }

        for (const auto &tile : path_data.tiles.data) {
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
            *z_value = std::max(*z_value, (unsigned int)draw_path_id);
            // ----------------------------------------------------------
        }
    }

    return draw_tile_batch;
}

void SceneBuilderD3D9::build(const std::shared_ptr<Driver> &driver) {
    // No need to rebuild the scene if it hasn't changed.
    // Comment this to do benchmark more precisely.
    if (!scene->is_dirty) return;

    // Build paint data.
    auto paint_metadata = scene->palette.build_paint_info(driver);

    // Most important step. Build draw paths into built draw paths.
    auto built_paths = build_paths_on_cpu(paint_metadata);

    finish_building(built_paths);

    // Mark the scene as clean, so we don't need to rebuild it the next frame.
    scene->is_dirty = false;
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

    // We need to build clip paths first.
    std::vector<BuiltPath> built_clip_paths(clip_paths_count);
    {
        // Parallel build.
        auto task = [this, &built_clip_paths, &clip_paths_count, &view_box](int begin) {
            for (int path_index = begin; path_index < clip_paths_count; path_index += PATHFINDER_THREADS) {
                PathBuildParams params;
                params.path_id = path_index;
                params.view_box = view_box;
                params.scene = scene;

                built_clip_paths[path_index] = build_clip_path_on_cpu(params);
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

    std::vector<BuiltDrawPath> built_draw_paths(draw_paths_count);
    {
        // Parallel build.
        auto task =
            [this, &built_draw_paths, &draw_paths_count, &view_box, &paint_metadata, &built_clip_paths](int begin) {
                for (int path_index = begin; path_index < draw_paths_count; path_index += PATHFINDER_THREADS) {
                    auto &draw_path = scene->draw_paths[path_index];

                    // Skip invisible draw paths.
                    if (!scene->get_paint(draw_path.paint).is_opaque() ||
                        !draw_path.outline.bounds.intersects(scene->get_view_box())) {
                        continue;
                    }

                    DrawPathBuildParams params;
                    params.paint_metadata = paint_metadata;
                    params.built_clip_paths = built_clip_paths;
                    params.path_build_params.path_id = path_index;
                    params.path_build_params.view_box = view_box;
                    params.path_build_params.scene = scene;

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

    return built_draw_paths;
}

BuiltPath SceneBuilderD3D9::build_clip_path_on_cpu(const PathBuildParams &params) {
    uint32_t path_id = params.path_id;

    const auto &path_object = scene->clip_paths[path_id];

    TilingPathInfo path_info;
    path_info.type = TilingPathInfo::Type::Clip;

    // Create a tiler for the draw path.
    Tiler tiler(*this,
                path_id,
                path_object.outline,
                path_object.fill_rule,
                scene->get_view_box(),
                path_object.clip_path,
                {},
                path_info);

    // Core step.
    tiler.generate_tiles();

    // Add generated fills from the tile generation step. Need a lock.
    fill_write_mutex.lock();
    send_fills(tiler.object_builder.fills);
    fill_write_mutex.unlock();

    return tiler.object_builder.built_path;
}

BuiltDrawPath SceneBuilderD3D9::build_draw_path_on_cpu(const DrawPathBuildParams &params) {
    uint32_t path_id = params.path_build_params.path_id;

    // Get the draw path (a thin wrapper over outline).
    const auto &path_object = scene->draw_paths[path_id];

    TilingPathInfo path_info;
    path_info.type = TilingPathInfo::Type::Draw;
    path_info.info.paint_id = path_object.paint;
    path_info.info.blend_mode = path_object.blend_mode;
    path_info.info.fill_rule = path_object.fill_rule;

    // Create a tiler for the draw path.
    Tiler tiler(*this,
                path_id,
                path_object.outline,
                path_object.fill_rule,
                scene->get_view_box(),
                path_object.clip_path,
                {},
                path_info);

    // Core step.
    tiler.generate_tiles();

    //    tiler.object_builder.built_path.paint_id = path_object.paint;

    // Add generated fills from the tile generation step. Need a lock.
    fill_write_mutex.lock();
    send_fills(tiler.object_builder.fills);
    fill_write_mutex.unlock();

    return {tiler.object_builder.built_path, path_object.blend_mode, path_object.fill_rule, true};
}

void SceneBuilderD3D9::build_tile_batches(const std::vector<BuiltDrawPath> &built_paths) {
    // Clear batches.
    tile_batches.clear();

    std::vector<RenderTarget> render_target_stack;

    // Build batches using the display items.
    for (const auto &display_item : scene->display_list) {
        switch (display_item.type) {
            case DisplayItem::Type::PushRenderTarget: {
                render_target_stack.push_back(display_item.render_target);
            } break;
            case DisplayItem::Type::PopRenderTarget: {
                render_target_stack.pop_back();
            } break;
            case DisplayItem::Type::DrawPaths: {
                // Create a new batch.
                auto tile_batch =
                    build_tile_batches_for_draw_path_display_item(*scene, built_paths, display_item.path_range);

                // Set render target. Pick the one on the top of the stack.
                if (!render_target_stack.empty()) {
                    tile_batch.render_target = render_target_stack.back();
                }

                tile_batches.push_back(tile_batch);
            } break;
        }
    }
}

void SceneBuilderD3D9::send_fills(const std::vector<Fill> &fill_batch) {
    pending_fills.insert(pending_fills.end(), fill_batch.begin(), fill_batch.end());
}

} // namespace Pathfinder
