#include "platform.h"

#include "driver.h"
#include "swap_chain.h"
#include "../../common/logger.h"

#include <stdexcept>
#include <sstream>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    std::shared_ptr<Platform> Platform::new_impl(DeviceType device_type,
                                                 uint32_t window_width,
                                                 uint32_t window_height) {
        if (device_type == DeviceType::OpenGl4) {
            return std::make_shared<PlatformGl>(window_width, window_height);
        }

        abort();
    }

    PlatformGl::PlatformGl(uint32_t window_width, uint32_t window_height) {
        // Get a GLFW window.
        init_window(window_width, window_height);
    }

    void PlatformGl::init_window(uint32_t p_width, uint32_t p_height) {
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
        window = glfwCreateWindow(p_width, p_height, "Pathfinder Demo (OpenGL)", nullptr, nullptr);

        if (window == nullptr) {
            Logger::error("Failed to create GLFW window!", "GLFW");
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // GLAD: load all OpenGL function pointers.
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            Logger::error("Failed to initialize GLAD!", "GLAD");
            return;
        }
#endif

        // Print GL version.
        int glMajorVersion, glMinorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

        std::ostringstream string_stream;
        string_stream << "Version: " << glMajorVersion << '.' << glMinorVersion;
        Logger::info(string_stream.str(), "OpenGL");
    }

    void PlatformGl::cleanup() {
#ifndef __ANDROID__
        // GLFW: terminate, clearing all previously allocated resources (including windows).
        glfwTerminate();
#endif
    }

    std::shared_ptr<Driver> PlatformGl::create_driver() {
        auto driver = std::make_shared<Pathfinder::DriverGl>();
        return driver;
    }

    std::shared_ptr<SwapChain>
    PlatformGl::create_swap_chain(const std::shared_ptr<Driver> &driver, uint32_t p_width, uint32_t p_height) {
        auto swap_chain_gl = std::make_shared<SwapChainGl>(p_width, p_height, window);
        return swap_chain_gl;
    }
}

#endif
