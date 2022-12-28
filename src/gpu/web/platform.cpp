#include "platform.h"

#include <stdexcept>

#include "../../common/logger.h"
#include "../gl/driver.h"
#include "../gl/swap_chain.h"

#ifndef PATHFINDER_USE_VULKAN

    #define CLIENT_API GLFW_OPENGL_ES_API

namespace Pathfinder {

    #ifdef PATHFINDER_USE_WASM
std::shared_ptr<Platform> Platform::new_impl(DeviceType device_type, Vec2I _window_size) {
    if (device_type == DeviceType::WebGl2) {
        return std::make_shared<PlatformWebGl>(_window_size);
    }

    abort();
}
    #endif

PlatformWebGl::PlatformWebGl(Vec2I _window_size) : Platform(_window_size) {
    // Get a GLFW window.
    init_window();
}

void PlatformWebGl::init_window() {
    #ifndef __ANDROID__

    assert(glfwInit() == GL_TRUE);
    assert(!strcmp(glfwGetVersionString(), "3.2.1 JS WebGL Emscripten"));
    assert(glfwGetCurrentContext() == nullptr);

    {
        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        assert(major == 3);
        assert(minor == 2);
        assert(rev == 1);
    }

    {
        int count, x, y, w, h;
        GLFWmonitor **monitors = glfwGetMonitors(&count);
        assert(count == 1);
        for (int i = 0; i < count; ++i) {
            assert(monitors[i] != NULL);
        }

        assert(glfwGetPrimaryMonitor() != NULL);
        glfwGetMonitorPos(monitors[0], &x, &y);
        glfwGetMonitorPhysicalSize(monitors[0], &w, &h);
        assert(glfwGetMonitorName(monitors[0]) != NULL);
    }

    {
        int x, y, w, h;
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, CLIENT_API);

        window = glfwCreateWindow(window_size.x, window_size.y, "pathfinder", NULL, NULL);
        assert(window != NULL);

        glfwGetWindowPos(window, &x, &y);

        glfwGetWindowSize(window, &w, &h);
        assert(w == window_size.x && h == window_size.y);

        assert(glfwGetWindowAttrib(window, GLFW_CLIENT_API) == CLIENT_API);
    }

    #endif
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
