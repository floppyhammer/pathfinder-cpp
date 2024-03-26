#ifndef PATHFINDER_D3D11_SCENE_BUILDER_H
#define PATHFINDER_D3D11_SCENE_BUILDER_H

#include <vector>

#include "../scene_builder.h"
#include "gpu_data.h"

#ifdef PATHFINDER_ENABLE_D3D11

namespace Pathfinder {

struct BuiltSegments {
    SegmentsD3D11 draw_segments;
    SegmentsD3D11 clip_segments;

    std::vector<Range> draw_segment_ranges;
    std::vector<Range> clip_segment_ranges;

    static BuiltSegments from_scene(const Scene &scene) {
        BuiltSegments built_segments;

        built_segments.clip_segment_ranges.reserve(scene.clip_paths.size());

        for (const auto &clip_path : scene.clip_paths) {
            auto range = built_segments.clip_segments.add_path(clip_path.outline);
            built_segments.clip_segment_ranges.push_back(range);
        }

        built_segments.draw_segment_ranges.reserve(scene.draw_paths.size());

        for (const auto &draw_path : scene.draw_paths) {
            auto range = built_segments.draw_segments.add_path(draw_path.outline);
            built_segments.draw_segment_ranges.push_back(range);
        }

        return built_segments;
    }
};

struct ClipBatchesD3D11 {
    // Will be submitted in reverse (LIFO) order.
    std::vector<TileBatchDataD3D11> prepare_batches;
    std::unordered_map<uint32_t, uint32_t> clip_id_to_path_batch_index;
};

class SceneBuilderD3D11 : public SceneBuilder {
public:
    SceneBuilderD3D11() = default;

    BuiltSegments built_segments;

    std::shared_ptr<ClipBatchesD3D11> clip_batches_d3d11;

    // Will be sent to renderer to draw tiles.
    std::vector<DrawTileBatchD3D11> tile_batches;

    void build(Scene *_scene, Renderer *renderer) override;

private:
    void finish_building(LastSceneInfo &last_scene,
                         const std::vector<PaintMetadata> &paint_metadata,
                         const std::shared_ptr<std::vector<BuiltDrawPath>> &built_paths);

    void build_tile_batches(LastSceneInfo &last_scene,
                            const std::vector<PaintMetadata> &paint_metadata,
                            const std::shared_ptr<std::vector<BuiltDrawPath>> &built_paths);
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_D3D11_SCENE_BUILDER_H
