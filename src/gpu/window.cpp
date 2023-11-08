#include "window.h"

namespace Pathfinder {

Vec2I Window::get_size() const {
    return size;
}

bool Window::get_resize_flag() const {
    return just_resized;
}

bool Window::is_minimized() const {
    return minimized;
}

} // namespace Pathfinder
