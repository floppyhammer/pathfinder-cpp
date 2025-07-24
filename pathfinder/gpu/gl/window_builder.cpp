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
    Logger::debug("Supported extensions:");

    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    for (GLint i = 0; i < num_extensions; i++) {
        const auto extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
        Logger::debug(std::string(extension));
        if (!strcmp(name, extension)) {
            return true;
        }
    }
    return false;
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
    auto glfw_window = glfw_window_init(size, PRIMARY_WINDOW_TITLE, dpi_scaling_factor, false, nullptr);

    primary_window_ = std::make_shared<WindowGl>(size, glfw_window);
    primary_window_->set_dpi_scaling_factor(dpi_scaling_factor);

    // Set user data.
    primary_window_->window_index = 0;

    std::ostringstream ss;
    ss << "Window created:\n  Size: " << size << "\n  DPI Scaling: " << dpi_scaling_factor;
    Logger::info(ss.str());

    // Have to make the window context current before calling gladLoadGL().
    glfwMakeContextCurrent(glfw_window);

    #ifndef __EMSCRIPTEN__
    // GLAD: load all OpenGL function pointers.
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    if (GLAD_GL_EXT_debug_label) {
        Logger::info("Debug markers enabled.");
    } else {
        Logger::info("Debug markers disabled. Try running from inside a OpenGL graphics debugger (e.g. RenderDoc).");
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
    Logger::info(string_stream.str());
}

WindowBuilderGl::~WindowBuilderGl() {
    // Destroy windows.
    for (auto &w : sub_windows_) {
        // We need to destroy a window explicitly in case its smart pointer is held elsewhere.
        w->destroy();
    }
    sub_windows_.clear();

    primary_window_->destroy();
    primary_window_.reset();

    // Cleanup GLFW.
#ifndef __ANDROID__
    glfwTerminate();
#endif
}

uint8_t WindowBuilderGl::create_window(const Vec2I &size, const std::string &title) {
#ifndef __ANDROID__
    auto window_gl = (WindowGl *)primary_window_.get();

    float dpi_scaling_factor;
    auto glfw_window =
        glfw_window_init(size, title, dpi_scaling_factor, false, (GLFWwindow *)window_gl->get_glfw_handle());

    auto new_window = std::make_shared<WindowGl>(size, glfw_window);
    new_window->set_dpi_scaling_factor(dpi_scaling_factor);

    sub_windows_.push_back(new_window);

    new_window->window_index = sub_windows_.size();

    return sub_windows_.size();
#else
    return 0;
#endif
}

std::shared_ptr<Device> WindowBuilderGl::request_device() {
    return std::make_shared<DeviceGl>();
}

std::shared_ptr<Queue> WindowBuilderGl::create_queue() {
    return std::make_shared<QueueGl>();
}

} // namespace Pathfinder
