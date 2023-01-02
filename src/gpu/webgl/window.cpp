#include "window.h"

#include <stdexcept>

#include "../../common/logger.h"
#include "../gl/driver.h"
#include "../gl/swap_chain.h"

#ifdef __EMSCRIPTEN__

namespace Pathfinder {

std::shared_ptr<Window> Window::new_impl(DeviceType device_type, Vec2I _size) {
    if (device_type == DeviceType::WebGl2) {
        return std::make_shared<WindowWebGl>(_size);
    }

    abort();
}

WindowWebGl::WindowWebGl(Vec2I _size) : Window(_size) {
    // Get a GLFW window.
    init();
}

void WindowWebGl::init() {
    if (glfwInit() != GL_TRUE) {
        Logger::error("GLFW initialization is not OK!", "WindowWebGl");
    }

    Logger::error(std::string(glfwGetVersionString()), "WindowWebGl");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfw_window = glfwCreateWindow(size.x, size.y, "pathfinder", NULL, NULL);
    if (glfw_window == nullptr) {
        Logger::error("Failed to create a GLFW window!", "WindowWebGl");
    }

    glfwMakeContextCurrent(glfw_window);

    if (glfwGetCurrentContext() == nullptr) {
        Logger::error("Failed to make GL context current!", "WindowWebGl");
    }
}

void WindowWebGl::cleanup() {
    glfwTerminate();
}

std::shared_ptr<Driver> WindowWebGl::create_driver() {
    return std::make_shared<Pathfinder::DriverGl>();
}

std::shared_ptr<SwapChain> WindowWebGl::create_swap_chain(const std::shared_ptr<Driver> &driver) {
    return std::make_shared<SwapChainGl>(size, glfw_window);
}

} // namespace Pathfinder

#endif
