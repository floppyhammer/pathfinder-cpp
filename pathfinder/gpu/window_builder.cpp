#include "window_builder.h"

#include "window.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "vk/base.h"
#else
    #include "gl/base.h"
#endif

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
}

} // namespace Pathfinder
