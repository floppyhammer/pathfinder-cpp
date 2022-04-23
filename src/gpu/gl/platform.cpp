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

        driver = std::make_shared<Pathfinder::DriverGl>();
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

    void PlatformGl::handle_inputs() {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
//        // GLFW input callbacks.
//        {
//            // A lambda function that doesn't capture anything can be implicitly converted to a regular function pointer.
//            auto cursor_position_callback = [](GLFWwindow *window, double x_pos, double y_pos) {
//                Pathfinder::InputEvent input_event{};
//                input_event.type = Pathfinder::InputEventType::MouseMotion;
//                input_event.args.mouse_motion.position = {(float) x_pos, (float) y_pos};
//                InputServer::get_singleton().input_queue.push_back(input_event);
//                InputServer::get_singleton().cursor_position = {(float) x_pos, (float) y_pos};
//                Pathfinder::Logger::verbose("Cursor movement", "InputEvent");
//            };
//            glfwSetCursorPosCallback(platform.get_glfw_window(), cursor_position_callback);
//
//            auto cursor_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
//                Pathfinder::InputEvent input_event{};
//                input_event.type = Pathfinder::InputEventType::MouseButton;
//                input_event.args.mouse_button.button = button;
//                input_event.args.mouse_button.pressed = action == GLFW_PRESS;
//                input_event.args.mouse_button.position = InputServer::get_singleton().cursor_position;
//                InputServer::get_singleton().input_queue.push_back(input_event);
//                Pathfinder::Logger::verbose("Cursor button", "InputEvent");
//            };
//            glfwSetMouseButtonCallback(platform.get_glfw_window(), cursor_button_callback);
//        }
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
}

#endif
