#include "scene_builder.h"

#include "../../common/logger.h"
#include "../../common/timestamp.h"
#include "../data/built_path.h"
#include "gpu_data.h"

#ifdef PATHFINDER_USE_D3D11

using std::shared_ptr;
using std::vector;

namespace Pathfinder {

struct ClipBatchesD3D11 {
    // Will be submitted in reverse (LIFO) order.
    vector<TileBatchDataD3D11> prepare_batches;
    unordered_map<uint32_t, uint32_t> clip_id_to_path_batch_index;
};

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

BuiltDrawPath prepare_draw_path_for_gpu_binning(Scene &scene,
                                                uint32_t draw_path_id,
                                                const Transform2 &transform,
                                                const std::vector<PaintMetadata> &paint_metadata) {
    auto effective_view_box = scene.get_view_box();

    auto &draw_path = scene.draw_paths[draw_path_id];

    auto path_bounds = transform * draw_path.outline.bounds;

    auto paint_id = draw_path.paint;

    TilingPathInfo path_info{};
    path_info.type = TilingPathInfo::Type::Draw;
    path_info.info.paint_id = paint_id;

    auto built_path = BuiltPath(draw_path_id, path_bounds, effective_view_box, draw_path.fill_rule, nullptr, path_info);

    // FIXME: Fix hardcoded blend mode.
    return {built_path, BlendMode::SrcOver, draw_path.fill_rule, true};
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
        if (map.find(*clip_path_id) == map.end()) {
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
DrawTileBatchD3D11 build_tile_batches_for_draw_path_display_item(
    Scene &scene,
    Range draw_path_id_range,
    const std::vector<PaintMetadata> &paint_metadata,
    LastSceneInfo &last_scene,
    uint32_t &next_batch_id,
    const shared_ptr<ClipBatchesD3D11> &clip_batches_d3d11) {
    // New draw tile batch.
    DrawTileBatchD3D11 draw_tile_batch;

    // This is a temporary value for test.
    // FIXME: Fix hardcoded transform.
    auto transform = Transform2::from_translation(Vec2F(0, 0));

    // Create a new batch if necessary.
    draw_tile_batch.tile_batch_data = TileBatchDataD3D11(next_batch_id, PathSource::Draw);
    draw_tile_batch.metadata_texture = scene.palette.metadata_texture;
    draw_tile_batch.tile_batch_data.prepare_info.transform = transform;

    for (auto draw_path_id = draw_path_id_range.start; draw_path_id < draw_path_id_range.end; draw_path_id++) {
        auto draw_path = prepare_draw_path_for_gpu_binning(scene, draw_path_id, transform, paint_metadata);

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

        // Add clip path if necessary.

        shared_ptr<GlobalPathId> clip_path;
        if (clip_batches_d3d11) {
            clip_path =
                add_clip_path_to_batch(scene, draw_path.clip_path_id, 0, transform, last_scene, *clip_batches_d3d11);
        }

        draw_tile_batch.tile_batch_data.push(draw_path.path, draw_path_id, clip_path, draw_path.occludes, last_scene);
    }

    next_batch_id += 1;

    return draw_tile_batch;
}

void SceneBuilderD3D11::build(const std::shared_ptr<Driver> &driver) {
    built_segments = BuiltSegments::from_scene(*scene);

    // Build paint data.
    auto paint_metadata = scene->palette.build_paint_info(driver);

    auto last_scene = LastSceneInfo{
        scene->id,
        scene->epoch,
        built_segments.draw_segment_ranges,
    };

    shared_ptr<vector<BuiltDrawPath>> built_paths;

    finish_building(last_scene, paint_metadata, built_paths);
}

void SceneBuilderD3D11::build_tile_batches(LastSceneInfo &last_scene,
                                           const std::vector<PaintMetadata> &paint_metadata,
                                           const shared_ptr<vector<BuiltDrawPath>> &built_paths) {
    // Clear batches.
    tile_batches.clear();

    std::vector<RenderTarget> render_target_stack;

    uint32_t next_batch_id = 0;

    shared_ptr<ClipBatchesD3D11> clip_batches_d3d11;
    if (built_paths == nullptr) {
        clip_batches_d3d11 = std::make_shared<ClipBatchesD3D11>();
    }

    // Prepare display items.
    for (const auto &display_item : scene->display_list) {
        switch (display_item.type) {
            case DisplayItem::Type::PushRenderTarget: {
                render_target_stack.push_back(display_item.render_target);
            } break;
            case DisplayItem::Type::PopRenderTarget: {
                render_target_stack.pop_back();
            } break;
            case DisplayItem::Type::DrawPaths: {
                auto tile_batch = build_tile_batches_for_draw_path_display_item(*scene,
                                                                                display_item.path_range,
                                                                                paint_metadata,
                                                                                last_scene,
                                                                                next_batch_id,
                                                                                clip_batches_d3d11);

                // Set render target. Render to screen if there's no render targets on the stack.
                if (!render_target_stack.empty()) {
                    tile_batch.render_target = render_target_stack.back();
                }

                tile_batches.push_back(tile_batch);
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
