#include "window_builder.h"

#ifndef __ANDROID__
    #include "render_api.h"
#endif
#ifdef PATHFINDER_USE_OPENGL
    #include "gl/window_builder.h"
#endif
#ifdef PATHFINDER_USE_VULKAN
    #include "vk/window_builder.h"
#endif
#include "window.h"

namespace Pathfinder {

std::shared_ptr<WindowBuilder> WindowBuilder::new_impl(BackendType backend_type, const Vec2I &size) {
#ifndef __ANDROID__
    switch (backend_type) {
    #ifdef PATHFINDER_USE_OPENGL
        case BackendType::Opengl: {
            Logger::info("Using OpenGL backend");
            return std::make_shared<WindowBuilderGl>(size);
        }
    #endif
    #ifdef PATHFINDER_USE_VULKAN
        case BackendType::Vulkan: {
            Logger::info("Using Vulkan backend");
            return std::make_shared<WindowBuilderVk>(size);
        }
    #endif
        default:
            abort();
    }
#else
    return nullptr;
#endif
}

std::weak_ptr<Window> WindowBuilder::get_window(uint8_t window_index) const {
    if (window_index == 0) {
        return primary_window_;
    }

    if (window_index <= sub_windows_.size()) {
        return sub_windows_[window_index - 1];
    }

    return primary_window_;
}

float WindowBuilder::get_dpi_scaling_factor(uint8_t window_index) const {
    return get_window(window_index).lock()->get_dpi_scaling_factor();
}

void WindowBuilder::poll_events() {
    // Reset window flags.
    {
        primary_window_->just_resized_ = false;

        for (auto w : sub_windows_) {
            w->just_resized_ = false;
        }
    }

#ifndef __ANDROID__
    glfwPollEvents();
#endif
}

void WindowBuilder::set_fullscreen(bool fullscreen) {
#ifndef __ANDROID__
    if (primary_window_->fullscreen_ == fullscreen) {
        return;
    }

    primary_window_->fullscreen_ = fullscreen;

    if (fullscreen) {
        reserved_window_logical_size_ = primary_window_->get_logical_size();
        reserved_window_position_ = primary_window_->get_position();

        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        auto physical_size = Vec2I(mode->width, mode->height);

        glfwSetWindowMonitor(primary_window_->glfw_window_,
                             glfwGetPrimaryMonitor(),
                             0,
                             0,
                             physical_size.x,
                             physical_size.y,
                             GLFW_DONT_CARE);

        auto logical_size = (physical_size.to_f32() / get_dpi_scaling_factor(0)).to_i32();
        primary_window_->logical_size_ = logical_size;
    } else {
        auto physical_size = (reserved_window_logical_size_.to_f32() * get_dpi_scaling_factor(0)).to_i32();

        glfwSetWindowMonitor(primary_window_->glfw_window_,
                             NULL,
                             reserved_window_position_.x,
                             reserved_window_position_.y,
                             physical_size.x,
                             physical_size.y,
                             GLFW_DONT_CARE);

        primary_window_->logical_size_ = reserved_window_logical_size_;
    }
#endif
}

#ifndef __ANDROID__
GLFWwindow *WindowBuilder::glfw_window_init(const Vec2I &logical_size,
                                            const std::string &title,
                                            float &dpi_scaling_factor,
                                            bool fullscreen,
                                            GLFWwindow *shared_window) {
    #ifndef __EMSCRIPTEN__
    // Enable window resizing.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Hide window upon creation as we need to center the window before showing it.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Get monitor position (used to correctly center the window in a multi-monitor scenario).
    int monitor_count, monitor_x, monitor_y;
    GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);

    const GLFWvidmode *video_mode = glfwGetVideoMode(monitors[0]);
    glfwGetMonitorPos(monitors[0], &monitor_x, &monitor_y);

    // Get DPI scale.
    float dpi_scale_x, dpi_scale_y;
    glfwGetMonitorContentScale(monitors[0], &dpi_scale_x, &dpi_scale_y);
    assert(dpi_scale_x == dpi_scale_y);
    dpi_scaling_factor = dpi_scale_x;
    #endif

    #if defined(__linux__) || defined(_WIN32)
    auto physical_size = (logical_size.to_f32() * dpi_scaling_factor).to_i32();
    #elif defined(__APPLE__)
    auto physical_size = logical_size;
    #endif

    auto glfw_window = glfwCreateWindow(physical_size.x,
                                        physical_size.y,
                                        title.c_str(),
                                        fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                                        shared_window);
    if (glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    #ifndef __EMSCRIPTEN__
    // Center the window.
    glfwSetWindowPos(glfw_window,
                     monitor_x + (video_mode->width - physical_size.x) / 2,
                     monitor_y + (video_mode->height - physical_size.y) / 2);

    // Show the window.
    glfwShowWindow(glfw_window);
    #endif

    return glfw_window;
}
#endif

} // namespace Pathfinder
