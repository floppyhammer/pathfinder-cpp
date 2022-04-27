#include "platform.h"

#include "../../common/logger.h"

#include <stdexcept>
#include <set>
#include <sstream>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    void PlatformGl::init(uint32_t p_width, uint32_t p_height) {
        // Get a GLFW window.
        initWindow(p_width, p_height);
    }

    void PlatformGl::initWindow(uint32_t p_width, uint32_t p_height) {
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
        window = glfwCreateWindow(p_width, p_height, "Pathfinder Demo (GL)", nullptr, nullptr);

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

        // Print GL version.
        int glMajorVersion, glMinorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

        std::ostringstream string_stream;
        string_stream << "Version: " << glMajorVersion << '.' << glMinorVersion;
        Logger::info(string_stream.str(), "OpenGL");
    }

    void PlatformGl::swap_buffers_and_poll_events() const {
        // GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.).
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    void PlatformGl::cleanup() {
        // GLFW: terminate, clearing all previously allocated resources (including windows).
        glfwTerminate();
    }

    std::shared_ptr<Driver> PlatformGl::create_driver() {
        auto driver = std::make_shared<Pathfinder::DriverGl>();
        return driver;
    }

    std::shared_ptr<SwapChain> PlatformGl::create_swap_chain(uint32_t p_width, uint32_t p_height) {
        auto swap_chain_gl = std::make_shared<SwapChainGl>(p_width, p_height);

        return swap_chain_gl;
    }
}

#endif
