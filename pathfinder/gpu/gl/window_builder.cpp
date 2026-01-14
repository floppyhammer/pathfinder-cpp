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

#ifndef __ANDROID__
WindowBuilderGl::WindowBuilderGl(const Vec2I &logical_size) {
    glfwInit();

    #if (defined(__linux__) && defined(__ARM_ARCH))
        // Set the desired OpenGL ES version.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        #ifdef PATHFINDER_ENABLE_D3D11
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        #else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        #endif
    #else
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
    #endif

    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(logical_size, PRIMARY_WINDOW_TITLE, dpi_scaling_factor, false, nullptr);

    auto physical_size = (logical_size.to_f32() * dpi_scaling_factor).to_i32();

    primary_window_ = std::make_shared<WindowGl>(physical_size, glfw_window);
    primary_window_->set_dpi_scaling_factor(dpi_scaling_factor);

    // Set user data.
    primary_window_->window_index = 0;

    std::ostringstream ss;
    ss << "Window created:\n  Physical Size: " << physical_size << "\n  Logical Size: " << logical_size
       << "\n  DPI Scaling: " << dpi_scaling_factor;
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

    // Print GL version.
    int gl_major_version, gl_minor_version;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);

    std::ostringstream string_stream;
    string_stream << "Version: " << gl_major_version << '.' << gl_minor_version;
    Logger::info(string_stream.str());
}

#else
WindowBuilderGl::WindowBuilderGl(ANativeWindow *native_window, const Vec2I &window_size) {
    egl_display_ = EGL_NO_DISPLAY;
    egl_surface_ = EGL_NO_SURFACE;
    egl_context_ = EGL_NO_CONTEXT;
    egl_config_ = nullptr;

    native_window_ = native_window;

    init_display();
    init_surface();
    init_context();

    // This is required.
    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);

    primary_window_ = std::make_shared<WindowGl>(window_size, egl_display_, egl_surface_, egl_context_);

    // Print GL version.
    int gl_major_version, gl_minor_version;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);

    std::ostringstream string_stream;
    string_stream << "Version: " << gl_major_version << '.' << gl_minor_version;
    Logger::info(string_stream.str());
}

bool WindowBuilderGl::init_display() {
    if (egl_display_ != EGL_NO_DISPLAY) {
        return true;
    }

    egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_FALSE == eglInitialize(egl_display_, nullptr, nullptr)) {
        Logger::error("NativeEngine: failed to init display");
        return false;
    }
    return true;
}

bool WindowBuilderGl::init_surface() {
    assert(egl_display_ != EGL_NO_DISPLAY);
    if (egl_surface_ != EGL_NO_SURFACE) {
        return true;
    }

    EGLint numConfigs;
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT, // Request OpenGL ES 3.0
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              16,
                              EGL_NONE};

    // Pick the first EGLConfig that matches.
    eglChooseConfig(egl_display_, attribs, &egl_config_, 1, &numConfigs);
    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config_, native_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        Logger::error("Failed to create EGL surface");
        return false;
    }
    return true;
}

bool WindowBuilderGl::init_context() {
    assert(egl_display_ != EGL_NO_DISPLAY);
    if (egl_context_ != EGL_NO_CONTEXT) {
        return true;
    }

    EGLint attribList[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    egl_context_ = eglCreateContext(egl_display_, egl_config_, nullptr, attribList);
    if (egl_context_ == EGL_NO_CONTEXT) {
        Logger::error("Failed to create EGL context");
        return false;
    }
    return true;
}
#endif

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
