#include "window_builder.h"

namespace Pathfinder {

std::shared_ptr<Window> WindowBuilder::get_primary_window() const {
    return primary_window;
}

} // namespace Pathfinder
