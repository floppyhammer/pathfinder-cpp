#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "gl/driver.h"
#include "vk/driver.h"

namespace Pathfinder {
    class Platform {
    public:
        virtual std::shared_ptr<Driver> create_driver() = 0;

        virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver>& driver, uint32_t p_width, uint32_t p_height) = 0;

        inline GLFWwindow *get_glfw_window() const {
            return window;
        }

        /// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
        inline void poll_events() {
            glfwPollEvents();

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

        virtual void cleanup() = 0;

    protected:
        GLFWwindow *window;
    };
}

#endif //PATHFINDER_GPU_PLATFORM_H
