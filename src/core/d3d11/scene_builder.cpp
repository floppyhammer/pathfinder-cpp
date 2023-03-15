#include "scene_builder.h"

#include "../../common/logger.h"
#include "../../common/timestamp.h"
#include "../data/built_path.h"
#include "gpu_data.h"

#ifdef PATHFINDER_USE_D3D11

using std::shared_ptr;
using std::vector;

namespace Pathfinder {

struct PreparedClipPath {
    BuiltPath built_path;
    shared_ptr<GlobalPathId> subclip_id;
};

// Forward declaration.
shared_ptr<GlobalPathId> add_clip_path_to_batch(Scene &scene,
                                                const shared_ptr<uint32_t> &clip_path_id,
                                                uint32_t clip_level,
                                                const Transform2 &transform,
                                                LastSceneInfo &last_scene,
                                                ClipBatchesD3D11 &clip_batches_d3d11);

std::shared_ptr<BuiltDrawPath> prepare_draw_path_for_gpu_binning(Scene &scene,
                                                                 uint32_t draw_path_id,
                                                                 const Transform2 &transform,
                                                                 const std::vector<PaintMetadata> &paint_metadata) {
    auto effective_view_box = scene.get_view_box();

    auto &draw_path = scene.draw_paths[draw_path_id];

    auto path_bounds = transform * draw_path.outline.bounds;

    // Clip the draw path by the view box.
    auto intersection = path_bounds.intersection(effective_view_box);

    if (intersection.is_valid()) {
        path_bounds = intersection;
    } else {
        return nullptr;
    }

    auto paint_id = draw_path.paint;
    auto &_paint_metadata = paint_metadata[paint_id];

    TilingPathInfo path_info{};
    path_info.type = TilingPathInfo::Type::Draw;
    path_info.info.paint_id = paint_id;

    auto built_path =
        BuiltPath(draw_path_id, path_bounds, effective_view_box, draw_path.fill_rule, draw_path.clip_path, path_info);

    return std::make_shared<BuiltDrawPath>(built_path, draw_path, _paint_metadata);
}

PreparedClipPath prepare_clip_path_for_gpu_binning(Scene &scene,
                                                   uint32_t clip_path_id,
                                                   const Transform2 &transform,
                                                   LastSceneInfo &last_scene,
                                                   size_t clip_level,
                                                   ClipBatchesD3D11 &clip_batches_d3d11) {
    auto effective_view_box = scene.get_view_box();
    auto clip_path = scene.clip_paths[clip_path_id];

    // Add subclip path if necessary.
    auto subclip_id =
        add_clip_path_to_batch(scene, clip_path.clip_path, clip_level + 1, transform, last_scene, clip_batches_d3d11);

    auto path_bounds = transform * clip_path.outline.bounds;

    // TODO(pcwalton): Clip to view box!

    TilingPathInfo path_info{};
    path_info.type = TilingPathInfo::Type::Clip;

    auto built_path =
        BuiltPath(clip_path_id, path_bounds, effective_view_box, clip_path.fill_rule, clip_path.clip_path, path_info);

    return PreparedClipPath{built_path, subclip_id};
}

shared_ptr<GlobalPathId> add_clip_path_to_batch(Scene &scene,
                                                const shared_ptr<uint32_t> &clip_path_id,
                                                uint32_t clip_level,
                                                const Transform2 &transform,
                                                LastSceneInfo &last_scene,
                                                ClipBatchesD3D11 &clip_batches_d3d11) {
    if (clip_path_id) {
        auto &map = clip_batches_d3d11.clip_id_to_path_batch_index;

        if (map.find(*clip_path_id) != map.end()) {
            auto clip_path_batch_index = map[*clip_path_id];
            return std::make_shared<GlobalPathId>(GlobalPathId{clip_level, clip_path_batch_index});
        } else {
            auto prepared_clip_path = prepare_clip_path_for_gpu_binning(scene,
                                                                        *clip_path_id,
                                                                        transform,
                                                                        last_scene,
                                                                        clip_level,
                                                                        clip_batches_d3d11);

            auto clip_path = prepared_clip_path.built_path;
            auto subclip_id = prepared_clip_path.subclip_id;

            while (clip_level >= clip_batches_d3d11.prepare_batches.size()) {
                auto clip_tile_batch_id = clip_batches_d3d11.prepare_batches.size();
                clip_batches_d3d11.prepare_batches.emplace_back(clip_tile_batch_id, PathSource::Clip);
            }

            auto clip_path_batch_index = clip_batches_d3d11.prepare_batches[clip_level].push(clip_path,
                                                                                             *clip_path_id,
                                                                                             subclip_id,
                                                                                             true,
                                                                                             last_scene);

            map[*clip_path_id] = clip_path_batch_index;

            return std::make_shared<GlobalPathId>(GlobalPathId{clip_level, clip_path_batch_index});
        }
    }

    return nullptr;
}

/// Create tile batches.
vector<DrawTileBatchD3D11> build_tile_batches_for_draw_path_display_item(
    Scene &scene,
    Range draw_path_id_range,
    const std::vector<PaintMetadata> &paint_metadata,
    LastSceneInfo &last_scene,
    uint32_t &next_batch_id,
    const shared_ptr<ClipBatchesD3D11> &clip_batches_d3d11) {
    vector<DrawTileBatchD3D11> flushed_draw_tile_batches;

    // New draw tile batch.
    shared_ptr<DrawTileBatchD3D11> draw_tile_batch;

    for (auto draw_path_id = draw_path_id_range.start; draw_path_id < draw_path_id_range.end; draw_path_id++) {
        // FIXME: This is a temporary value for test.
        auto transform = Transform2::from_translation(Vec2F(0, 0));

        auto draw_path = prepare_draw_path_for_gpu_binning(scene, draw_path_id, transform, paint_metadata);

        // Skip if the draw path is outside the view box.
        if (draw_path == nullptr) {
            continue;
        }

        // If we should create a new batch.
        bool flush_needed = false;

        // Try to reuse the current batch if we can.
        if (draw_tile_batch) {
            flush_needed = !fixup_batch_for_new_path_if_possible(draw_tile_batch->color_texture, *draw_path);
        }

        // If we couldn't reuse the batch, flush it.
        if (flush_needed) {
            flushed_draw_tile_batches.push_back(*draw_tile_batch);
            draw_tile_batch = nullptr;
        }

        if (draw_tile_batch == nullptr) {
            draw_tile_batch = std::make_shared<DrawTileBatchD3D11>();

            draw_tile_batch->tile_batch_data = TileBatchDataD3D11(next_batch_id, PathSource::Draw);
            draw_tile_batch->metadata_texture = scene.palette.get_metadata_texture();
            draw_tile_batch->tile_batch_data.prepare_info.transform = transform;
            draw_tile_batch->color_texture = draw_path->color_texture;

            next_batch_id += 1;
        }

        // Add clip path if necessary.
        shared_ptr<GlobalPathId> clip_path;
        if (clip_batches_d3d11) {
            clip_path =
                add_clip_path_to_batch(scene, draw_path->clip_path_id, 0, transform, last_scene, *clip_batches_d3d11);
        }

        draw_tile_batch->tile_batch_data.push(draw_path->path,
                                              draw_path_id,
                                              clip_path,
                                              draw_path->occludes,
                                              last_scene);
    }

    if (draw_tile_batch) {
        flushed_draw_tile_batches.push_back(*draw_tile_batch);
    }

    return flushed_draw_tile_batches;
}

void SceneBuilderD3D11::build(const std::shared_ptr<Driver> &driver) {
    built_segments = BuiltSegments::from_scene(*scene);

    // Build paint data.
    auto paint_metadata = scene->palette.build_paint_info(driver);

    auto last_scene =
        LastSceneInfo{scene->id, scene->epoch, built_segments.draw_segment_ranges, built_segments.clip_segment_ranges};

    shared_ptr<vector<BuiltDrawPath>> built_paths;

    finish_building(last_scene, paint_metadata, built_paths);
}

void SceneBuilderD3D11::build_tile_batches(LastSceneInfo &last_scene,
                                           const std::vector<PaintMetadata> &paint_metadata,
                                           const shared_ptr<vector<BuiltDrawPath>> &built_paths) {
    // Clear batches.
    tile_batches.clear();

    std::vector<RenderTargetId> render_target_stack;

    uint32_t next_batch_id = 0;

    if (built_paths == nullptr) { // Always holds true.
        clip_batches_d3d11 = std::make_shared<ClipBatchesD3D11>();
    }

    // Prepare display items.
    for (const auto &display_item : scene->display_list) {
        switch (display_item.type) {
            case DisplayItem::Type::PushRenderTarget: {
                render_target_stack.push_back(display_item.render_target_id);
            } break;
            case DisplayItem::Type::PopRenderTarget: {
                render_target_stack.pop_back();
            } break;
            case DisplayItem::Type::DrawPaths: {
                auto batches = build_tile_batches_for_draw_path_display_item(*scene,
                                                                             display_item.range,
                                                                             paint_metadata,
                                                                             last_scene,
                                                                             next_batch_id,
                                                                             clip_batches_d3d11);

                for (auto &batch : batches) {
                    // Set render target. Render to screen if there's no render targets on the stack.
                    if (!render_target_stack.empty()) {
                        auto render_target = scene->palette.get_render_target(render_target_stack.back());
                        batch.render_target = render_target;
                    }

                    tile_batches.push_back(batch);
                }
            } break;
        }
    }
}

void SceneBuilderD3D11::finish_building(LastSceneInfo &last_scene,
                                        const std::vector<PaintMetadata> &paint_metadata,
                                        const shared_ptr<vector<BuiltDrawPath>> &built_paths) {
    build_tile_batches(last_scene, paint_metadata, built_paths);
}

} // namespace Pathfinder

#endif
