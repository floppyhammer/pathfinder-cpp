#ifndef PATHFINDER_D3D11_SCENE_BUILDER_H
#define PATHFINDER_D3D11_SCENE_BUILDER_H

#include <vector>

#include "../scene_builder.h"
#include "gpu_data.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {

struct BuiltSegments {
    SegmentsD3D11 draw_segments;
    SegmentsD3D11 clip_segments;

    std::vector<Range> draw_segment_ranges;
    std::vector<Range> clip_segment_ranges;

    static BuiltSegments from_scene(Scene &scene) {
        BuiltSegments built_segments;
        built_segments.draw_segment_ranges.reserve(scene.draw_paths.size());

        for (const auto &draw_path : scene.draw_paths) {
            auto range = built_segments.draw_segments.add_path(draw_path.outline);
            built_segments.draw_segment_ranges.push_back(range);
        }

        return built_segments;
    }
};

class SceneBuilderD3D11 : public SceneBuilder {
public:
    explicit SceneBuilderD3D11(Scene *p_scene) : SceneBuilder(p_scene) {}

    BuiltSegments built_segments;

    // Sent to renderer to draw tiles.
    std::vector<DrawTileBatchD3D11> tile_batches;

    void build(const std::shared_ptr<Driver> &driver) override;

private:
    void finish_building(LastSceneInfo &last_scene, const std::vector<PaintMetadata> &paint_metadata);

    void build_tile_batches(LastSceneInfo &last_scene, const std::vector<PaintMetadata> &paint_metadata);
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_D3D11_SCENE_BUILDER_H
