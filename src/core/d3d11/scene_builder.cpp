#include "scene_builder.h"

#include "gpu_data.h"
#include "../d3dx/data/built_path.h"
#include "../../common/timestamp.h"
#include "../../common/logger.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    BuiltDrawPath prepare_draw_path_for_gpu_binning(
            Scene &scene,
            uint32_t draw_path_id,
            Transform2 &transform,
            const std::vector<TextureMetadataEntry> &paint_metadata) {
        auto effective_view_box = scene.view_box;

        auto &draw_path = scene.draw_paths[draw_path_id];

        auto path_bounds = transform * draw_path.outline.bounds;

        auto paint_id = draw_path.paint;

        auto &paint_metadata0 = paint_metadata[paint_id];

        auto built_path = BuiltPath(draw_path_id, path_bounds, paint_id, effective_view_box, draw_path.fill_rule);

        // FIXME: Fix hardcoded blend mode.
        return {built_path, BlendMode::SrcOver, draw_path.fill_rule, true};
    }

    /// Create tile batches.
    DrawTileBatchD3D11 build_tile_batches_for_draw_path_display_item(
            Scene &scene,
            Range draw_path_id_range,
            const std::vector<TextureMetadataEntry> &paint_metadata,
            LastSceneInfo &last_scene,
            uint32_t next_batch_id) {
        // New draw tile batch.
        DrawTileBatchD3D11 draw_tile_batch;

        // This is a temporary value for test.
        // FIXME: Fix hardcoded transform.
        auto transform = Transform2::from_translation(Vec2<float>(0, 0));

        // Create a new batch if necessary.
        draw_tile_batch.tile_batch_data = TileBatchDataD3D11(next_batch_id, PathSource::Draw);

        //draw_tile_batch.color_texture = draw_path.color_texture;
        draw_tile_batch.tile_batch_data.prepare_info.transform = transform;

        for (auto draw_path_id = draw_path_id_range.start; draw_path_id < draw_path_id_range.end; draw_path_id++) {
            auto draw_path = prepare_draw_path_for_gpu_binning(
                    scene,
                    draw_path_id,
                    transform,
                    paint_metadata);

            draw_tile_batch.tile_batch_data.push(draw_path.path,
                                                 draw_path_id,
                                                 draw_path.occludes,
                                                 last_scene);
        }

        return draw_tile_batch;
    }

    void SceneBuilderD3D11::build() {
        Timestamp timestamp;

        auto draw_path_count = scene->draw_paths.size();

        // Get metadata.
        metadata = scene->palette.build_paint_info();

        built_segments = BuiltSegments::from_scene(*scene);

        auto last_scene = LastSceneInfo{
                scene->id,
                scene->epoch,
                built_segments.draw_segment_ranges,
        };

        timestamp.record("SceneBuilderD3D11::build > from_scene");

        finish_building(last_scene);

        timestamp.record("SceneBuilderD3D11::build > finish_building");
        timestamp.print();

        Logger::verbose("build > draw path count " + std::to_string(draw_path_count), "SceneBuilderD3D11");
        Logger::verbose("build > tile batch count " + std::to_string(tile_batches.size()), "SceneBuilderD3D11");
    }

    void SceneBuilderD3D11::build_tile_batches(LastSceneInfo &last_scene) {
        // Clear batches.
        tile_batches.clear();

        std::vector<RenderTarget> render_target_stack;

        uint32_t next_batch_id = 0;

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
                    auto tile_batch = build_tile_batches_for_draw_path_display_item(
                            *scene,
                            display_item.path_range,
                            metadata,
                            last_scene,
                            next_batch_id);

                    next_batch_id += 1;

                    auto paint_id = scene->draw_paths[display_item.path_range.start].paint;

                    Paint paint = scene->palette.get_paint(paint_id);

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

                    // Set render target. Render to screen if there's no render targets on the stack.
                    if (!render_target_stack.empty()) {
                        tile_batch.render_target = render_target_stack.back();
                    }

                    tile_batches.push_back(tile_batch);
                }
                    break;
            }
        }
    }

    void SceneBuilderD3D11::finish_building(LastSceneInfo &last_scene) {
        build_tile_batches(last_scene);
    }
}

#endif
