#include "window.h"

#include <stdexcept>

#include "../../common/logger.h"
#include "../gl/driver.h"
#include "../gl/swap_chain.h"

#ifdef __EMSCRIPTEN__

namespace Pathfinder {

std::shared_ptr<Window> Window::new_impl(Vec2I _size) {
    return std::make_shared<WindowWebGl>(_size);
}

WindowWebGl::WindowWebGl(Vec2I _size) : Window(_size) {
    // Get a GLFW window.
    init();
}

void WindowWebGl::init() {
    if (glfwInit() != GL_TRUE) {
        Logger::error("GLFW initialization is not OK!", "WindowWebGl");
    }

    Logger::info(std::string(glfwGetVersionString()), "WindowWebGl");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfw_window = glfwCreateWindow(size.x, size.y, "Pathfinder", NULL, NULL);
    if (glfw_window == nullptr) {
        Logger::error("Failed to create GLFW window!", "WindowWebGl");
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
