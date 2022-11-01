#include "renderer.h"
#include "scene.h"

using std::shared_ptr;

namespace Pathfinder {

bool fixup_batch_for_new_path_if_possible(shared_ptr<Texture> &batch_color_texture, const BuiltDrawPath &draw_path) {
    if (draw_path.color_texture) {
        // If the current batch doesn't have a color texture.
        if (batch_color_texture == nullptr) {
            // Update batch color texture.
            batch_color_texture = draw_path.color_texture;
            return true;
        }

        // If the current batch has a different color texture than that of the draw path.
        if (draw_path.color_texture != batch_color_texture) {
            return false;
        }
    }
    return true;
}

} // namespace Pathfinder
