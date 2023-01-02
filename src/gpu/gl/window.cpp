#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "driver.h"
#include "swap_chain.h"

#ifndef PATHFINDER_USE_VULKAN

    #ifndef __EMSCRIPTEN__
        #define GLAD_GL_IMPLEMENTATION
        #include <glad/gl.h>

namespace Pathfinder {

std::shared_ptr<Window> Window::new_impl(DeviceType device_type, Vec2I _size) {
    if (device_type == DeviceType::OpenGl4) {
        return std::make_shared<WindowGl>(_size);
    }

    abort();
}

WindowGl::WindowGl(Vec2I _size) : Window(_size) {
    // Get a GLFW window.
    init();
}

void WindowGl::init() {
        #ifndef __ANDROID__
    // GLFW: initialize and configure.
    glfwInit();

            #ifdef PATHFINDER_USE_D3D11
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            #endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            #endif

    // GLFW: window creation.
    glfw_window = glfwCreateWindow(size.x, size.y, "Pathfinder (OpenGL)", nullptr, nullptr);

    if (glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    glfwMakeContextCurrent(glfw_window);

    // Set window resize callback.
    glfwSetWindowUserPointer(glfw_window, this);
    glfwSetFramebufferSizeCallback(glfw_window, framebuffer_resize_callback);

    // GLAD: load all OpenGL function pointers.
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    if (GLAD_GL_EXT_debug_label) {
        Logger::info("EXT_debug_label enabled.", "WindowGl");
    } else {
        Logger::info("EXT_debug_label disabled. Try to run from RenderDoc!", "WindowGl");
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

void WindowGl::cleanup() {
        #ifndef __ANDROID__
    // GLFW: terminate, clearing all previously allocated resources (including windows).
    glfwTerminate();
        #endif
}

std::shared_ptr<Driver> WindowGl::create_driver() {
    return std::make_shared<Pathfinder::DriverGl>();
}

std::shared_ptr<SwapChain> WindowGl::create_swap_chain(const std::shared_ptr<Driver> &driver) {
    return std::make_shared<SwapChainGl>(size, glfw_window);
}

} // namespace Pathfinder

    #endif

#endif
