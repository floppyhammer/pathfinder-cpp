//
// Created by floppyhammer on 6/25/2021.
//

#include "../../src/common/io.h"
#include "../../src/common/global_macros.h"
#include "../common/app.h"

#include "../../src/modules/vgui/ecs/coordinator.h"
#include "../../src/modules/vgui/ecs/components/components.h"
#include "../../src/modules/vgui/ecs/systems/physics_system.h"
#include "../../src/modules/vgui/ecs/systems/render_system.h"

#include <iostream>

using namespace Pathfinder;

class Window {
public:
    int width, height;

    Window(int p_width, int p_height);

    ~Window();

    GLFWwindow *get_glfw_window();

    void handle_inputs();

    void swap_buffers_and_poll_events();

private:
    GLFWwindow *glfw_window = nullptr;

    GLFWwindow *init_glfw_window() const;

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
};

int main() {
    // Create a window.
    Window window(1920, 1080);

    auto area_lut_input = load_file_as_bytes(PATHFINDER_RES_DIR"area-lut.png");
    auto font_input = load_file_as_bytes(PATHFINDER_RES_DIR"OpenSans-Regular.ttf");
    auto svg_input = load_file_as_string(PATHFINDER_RES_DIR"tiger.svg");

    App app(window.width, window.height, area_lut_input, font_input, svg_input);

    // ECS test.
    // ----------------------------------------------------------
    auto coordinator = Coordinator::get_singleton();

    coordinator.register_component<Gravity>();
    coordinator.register_component<RigidBody>();
    coordinator.register_component<Transform>();

    auto physics_system = coordinator.register_system<PhysicsSystem>();
    // Set signature for physics system.
    {
        Signature signature;
        signature.set(coordinator.get_component_type<Gravity>());
        signature.set(coordinator.get_component_type<RigidBody>());
        signature.set(coordinator.get_component_type<Transform>());
        coordinator.set_system_signature<PhysicsSystem>(signature);
    }

    auto render_system = coordinator.register_system<RenderSystem>();

    // Allocate space for entities.
    std::vector<Entity> entities(10);

    // Create some entities.
    for (auto &entity: entities) {
        entity = coordinator.create_entity();

        coordinator.add_component(
                entity,
                Gravity{Vec3<float>(0.0f, 0.0f, 0.0f)});

        coordinator.add_component(
                entity,
                RigidBody{
                        Vec3<float>(0.0f, 0.0f, 0.0f),
                        Vec3<float>(0.0f, 0.0f, 0.0f),
                });

        coordinator.add_component(
                entity,
                Transform{
                        Vec3<float>(0.0f, 0.0f, 0.0f),
                        Quaternion{0.0f, 0.0f, 0.0f, 0.0f},
                        Vec3<float>(0.0f, 0.0f, 0.0f),
                });
    }
    // ----------------------------------------------------------

    // Rendering loop.
    while (!glfwWindowShouldClose(window.get_glfw_window())) {
        window.handle_inputs();

        physics_system->update(0.001f);

        app.loop();

        window.swap_buffers_and_poll_events();
    }

    return 0;
}

Window::Window(int p_width, int p_height) : width(p_width), height(p_height) {
    glfw_window = init_glfw_window();
}

Window::~Window() {
    // GLFW: terminate, clearing all previously allocated resources (including windows).
    glfwTerminate();
}

GLFWwindow *Window::init_glfw_window() const {
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
    GLFWwindow *window = glfwCreateWindow(width, height, "Pathfinder Demo", nullptr, nullptr);

    if (window == nullptr) {
        Logger::error("Failed to create GLFW window!", "GLFW");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD: load all OpenGL function pointers.
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        Logger::error("Failed to initialize GLAD!", "GLAD");
        return nullptr;
    }

    // Print GL version.
    int glMajorVersion, glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

    std::ostringstream string_stream;
    string_stream << "Version: " << glMajorVersion << '.' << glMinorVersion;
    Logger::info(string_stream.str(), "OpenGL");

    return window;
}

/// GLFW: whenever the window size changed (by OS or user) this callback function executes.
void Window::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // Make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

GLFWwindow *Window::get_glfw_window() {
    return glfw_window;
}

/// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
void Window::handle_inputs() {
    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfw_window, true);
}

void Window::swap_buffers_and_poll_events() {
    // GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.).
    glfwSwapBuffers(glfw_window);
    glfwPollEvents();
}
