#include "renderer.h"
#include "scene.h"

namespace Pathfinder {

bool fixup_batch_for_new_path_if_possible(std::shared_ptr<TileBatchTextureInfo> &batch_color_texture,
                                          const BuiltDrawPath &draw_path) {
    if (draw_path.color_texture_info) {
        // If the current batch doesn't have a color texture.
        if (batch_color_texture == nullptr) {
            // Update batch color texture.
            batch_color_texture = draw_path.color_texture_info;
            return true;
        }

        // If the current batch has a different color texture than that of the draw path.
        if (*draw_path.color_texture_info != *batch_color_texture) {
            Logger::debug("Batch break: path color texture mismatches batch color texture!", "Scene Builder");
            return false;
        }
    }
    return true;
}

} // namespace Pathfinder
