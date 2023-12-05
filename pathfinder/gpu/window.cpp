#include "window.h"

namespace Pathfinder {

Vec2I Window::get_size() const {
    return size_;
}

bool Window::get_resize_flag() const {
    return just_resized_;
}

bool Window::is_minimized() const {
    return minimized_;
}

void* Window::get_raw_handle() const {
    return nullptr;
}

} // namespace Pathfinder
