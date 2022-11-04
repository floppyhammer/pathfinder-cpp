#include "platform.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "driver.h"
#include "swap_chain.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

std::shared_ptr<Platform> Platform::new_impl(DeviceType device_type, Vec2I _window_size) {
    if (device_type == DeviceType::OpenGl4) {
        return std::make_shared<PlatformGl>(_window_size);
    }

    abort();
}

PlatformGl::PlatformGl(Vec2I _window_size) : Platform(_window_size) {
    // Get a GLFW window.
    init_window();
}

void PlatformGl::init_window() {
    #ifndef __ANDROID__
    // GLFW: initialize and configure.
    glfwInit();

        #ifdef PATHFINDER_USE_D3D11
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        #endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

    // GLFW: window creation.
    window = glfwCreateWindow(window_size.x, window_size.y, "Pathfinder (OpenGL)", nullptr, nullptr);

    if (window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    glfwMakeContextCurrent(window);

    // Set window resize callback.
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    // GLAD: load all OpenGL function pointers.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
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

void PlatformGl::cleanup() {
    #ifndef __ANDROID__
    // GLFW: terminate, clearing all previously allocated resources (including windows).
    glfwTerminate();
    #endif
}

std::shared_ptr<Driver> PlatformGl::create_driver() {
    return std::make_shared<Pathfinder::DriverGl>();
}

std::shared_ptr<SwapChain> PlatformGl::create_swap_chain(const std::shared_ptr<Driver> &driver) {
    return std::make_shared<SwapChainGl>(window_size, window);
}

} // namespace Pathfinder

#endif
