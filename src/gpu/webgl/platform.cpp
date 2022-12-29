#include "platform.h"

#include <stdexcept>

#include "../../common/logger.h"
#include "../gl/driver.h"
#include "../gl/swap_chain.h"

#ifdef __EMSCRIPTEN__

namespace Pathfinder {

std::shared_ptr<Platform> Platform::new_impl(DeviceType device_type, Vec2I _window_size) {
    if (device_type == DeviceType::WebGl2) {
        return std::make_shared<PlatformWebGl>(_window_size);
    }

    abort();
}

PlatformWebGl::PlatformWebGl(Vec2I _window_size) : Platform(_window_size) {
    // Get a GLFW window.
    init_window();
}

void PlatformWebGl::init_window() {
    if (glfwInit() != GL_TRUE) {
        Logger::error("GLFW initialization is not OK!", "PlatformWebGl");
    }

    Logger::error(std::string(glfwGetVersionString()), "PlatformWebGl");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(window_size.x, window_size.y, "pathfinder", NULL, NULL);
    if (window == nullptr) {
        Logger::error("Failed to create a GLFW window!", "PlatformWebGl");
    }

    glfwMakeContextCurrent(window);

    if (glfwGetCurrentContext() == nullptr) {
        Logger::error("Failed to make GL context current!", "PlatformWebGl");
    }
}

void PlatformWebGl::cleanup() {
    glfwTerminate();
}

std::shared_ptr<Driver> PlatformWebGl::create_driver() {
    return std::make_shared<Pathfinder::DriverGl>();
}

std::shared_ptr<SwapChain> PlatformWebGl::create_swap_chain(const std::shared_ptr<Driver> &driver) {
    return std::make_shared<SwapChainGl>(window_size, window);
}

} // namespace Pathfinder

#endif
