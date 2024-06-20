#include "window_builder.h"

#include <cstring>
#include <memory>

#include "../../common/logger.h"
#include "base.h"
#include "device.h"
#include "queue.h"
#include "window.h"

namespace Pathfinder {

bool is_extension_supported(const char *name) {
    Logger::debug("Supported extensions:", "OpenGL");

    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    for (GLint i = 0; i < num_extensions; i++) {
        const auto extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
        Logger::debug(std::string(extension), "OpenGL");
        if (!strcmp(name, extension)) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<WindowBuilder> WindowBuilder::new_impl(const Vec2I &size) {
    return std::make_shared<WindowBuilderGl>(size);
}

WindowBuilderGl::WindowBuilderGl(const Vec2I &size) {
#ifndef __ANDROID__
    glfwInit();

    // Major GL version.
    #ifdef PATHFINDER_ENABLE_D3D11
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    #endif

    // Minor GL version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(size, PRIMARY_WINDOW_TITLE, dpi_scaling_factor);
    primary_window_ = std::make_shared<WindowGl>(size, glfw_window);
    primary_window_->set_dpi_scaling_factor(dpi_scaling_factor);

    // Have to make the window context current before calling gladLoadGL().
    glfwMakeContextCurrent(glfw_window);

    #ifndef __EMSCRIPTEN__
    // GLAD: load all OpenGL function pointers.
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    if (GLAD_GL_EXT_debug_label) {
        Logger::info("Debug markers enabled.", "WindowBuilderGl");
    } else {
        Logger::info("Debug markers disabled. Try running from inside a OpenGL graphics debugger (e.g. RenderDoc).",
                     "WindowBuilderGl");
    }
    #endif
#else
    primary_window_ = std::make_shared<WindowGl>(size);
#endif

    // Print GL version.
    int gl_major_version, gl_minor_version;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);

    std::ostringstream string_stream;
    string_stream << "Version: " << gl_major_version << '.' << gl_minor_version;
    Logger::info(string_stream.str(), "OpenGL");
}

WindowBuilderGl::~WindowBuilderGl() {
    // Destroy windows.
    {
        for (auto &w : sub_windows_) {
            if (!w.expired()) {
                // We need to destroy a window explicitly in case its smart pointer is held elsewhere.
                w.lock()->destroy();
            }
        }
        sub_windows_.clear();

        primary_window_->destroy();
        primary_window_.reset();
    }

#ifndef __ANDROID__
    glfwTerminate();
#endif
}

#ifndef __ANDROID__
GLFWwindow *WindowBuilderGl::glfw_window_init(const Vec2I &logical_size,
                                              const std::string &title,
                                              float &dpi_scaling_factor,
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
    #else ifdef __APPLE__
    auto physical_size = logical_size;
    #endif

    auto glfw_window = glfwCreateWindow(physical_size.x, physical_size.y, title.c_str(), nullptr, shared_window);
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

std::shared_ptr<Window> WindowBuilderGl::create_window(const Vec2I &size, const std::string &title) {
#ifndef __ANDROID__
    auto window_gl = (WindowGl *)primary_window_.get();

    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(size, title, dpi_scaling_factor, (GLFWwindow *)window_gl->get_glfw_handle());

    auto new_window = std::make_shared<WindowGl>(size, glfw_window);
    new_window->set_dpi_scaling_factor(dpi_scaling_factor);

    sub_windows_.push_back(new_window);

    return new_window;
#else
    return nullptr;
#endif
}

std::shared_ptr<Device> WindowBuilderGl::request_device() {
    return std::make_shared<DeviceGl>();
}

std::shared_ptr<Queue> WindowBuilderGl::create_queue() {
    return std::make_shared<QueueGl>();
}

} // namespace Pathfinder
