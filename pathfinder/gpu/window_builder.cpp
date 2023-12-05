#include "window_builder.h"

namespace Pathfinder {

std::shared_ptr<Window> WindowBuilder::get_main_window() const {
    return main_window;
}

} // namespace Pathfinder
