#include "window_builder.h"

#include "../../common/logger.h"
#include "device.h"
#include "queue.h"
#include "window.h"

#if (defined(WIN32) || defined(__linux__))
    #ifndef GLAD_GL_IMPLEMENTATION
        #define GLAD_GL_IMPLEMENTATION
    #endif
    #include <glad/gl.h>
#endif

namespace Pathfinder {

#ifndef __ANDROID__
std::shared_ptr<WindowBuilder> WindowBuilder::new_impl(const Vec2I& size) {
    return std::make_shared<WindowBuilderGl>(size);
}
#endif

WindowBuilderGl::WindowBuilderGl(const Vec2I& size) {
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

    auto glfw_window = common_glfw_window_init(size, "Main");
    main_window = std::make_shared<WindowGl>(size, glfw_window);

    // Have to make the window context current before calling gladLoadGL().
    glfwMakeContextCurrent(glfw_window);

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
        for (auto& w : sub_windows) {
            if (!w.expired()) {
                // We need to destroy a window explicitly in case its smart pointer is held elsewhere.
                auto window_vk = (WindowGl*)w.lock().get();
                window_vk->destroy();
            }
        }
        sub_windows.clear();

        auto window_vk = (WindowGl*)main_window.get();
        window_vk->destroy();
        main_window.reset();
    }

#ifndef __ANDROID__
    glfwTerminate();
#endif
}

#ifndef __ANDROID__
std::shared_ptr<Window> WindowBuilderGl::create_window(const Vec2I& size, const std::string& title) {
    auto glfw_window = common_glfw_window_init(size, title, main_window->get_glfw_window());

    auto new_window = std::make_shared<WindowGl>(size, glfw_window);
    sub_windows.push_back(new_window);

    return new_window;
}
#endif

std::shared_ptr<Device> WindowBuilderGl::request_device() {
    return std::make_shared<DeviceGl>();
}

std::shared_ptr<Queue> WindowBuilderGl::create_queue() {
    return std::make_shared<QueueGl>();
}

} // namespace Pathfinder
