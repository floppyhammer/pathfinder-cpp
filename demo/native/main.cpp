#include "../common/app.h"
#include "../src/gpu/gl/platform_gl.h"

using namespace Pathfinder;

uint32_t WINDOW_WIDTH = 1920;
uint32_t WINDOW_HEIGHT = 1080;

int main() {
    auto platform = PlatformGl::get_singleton();

    platform.init(WINDOW_WIDTH, WINDOW_HEIGHT);

    InputServer::get_singleton();

    auto area_lut_input = load_file_as_bytes(PATHFINDER_ASSET_DIR"area-lut.png");
    auto font_input = load_file_as_bytes(PATHFINDER_ASSET_DIR"OpenSans-Regular.ttf");
    auto svg_input = load_file_as_string(PATHFINDER_ASSET_DIR"tiger.svg");

    App app(WINDOW_WIDTH, WINDOW_HEIGHT, area_lut_input, font_input, svg_input);

    // Rendering loop.
    while (!glfwWindowShouldClose(platform.get_glfw_window())) {
        platform.handle_inputs();

        app.loop();

        platform.swap_buffers_and_poll_events();
    }

    platform.cleanup();

    return 0;
}
