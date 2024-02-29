#include "window_builder.h"

#ifndef __ANDROID__
    #include "GLFW/glfw3.h"
#endif

#include "window.h"

namespace Pathfinder {

std::shared_ptr<Window> WindowBuilder::get_primary_window() const {
    return primary_window_;
}

void WindowBuilder::poll_events() {
    // Reset window flags.
    {
        primary_window_->just_resized_ = false;

        for (auto w : sub_windows_) {
            if (!w.expired()) {
                w.lock()->just_resized_ = false;
            }
        }
    }

#ifndef __ANDROID__
    glfwPollEvents();
#endif

    // if (glfwGetKey(glfw_window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(glfw_window_, true);
    // }
}

} // namespace Pathfinder
