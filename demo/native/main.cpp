#include "../common/app.h"
#include "../src/gpu/gl/platform.h"
#include "../src/gpu/vk/platform.h"
#include "../src/gpu/vk/swap_chain.h"

using namespace Pathfinder;

uint32_t WINDOW_WIDTH = 1920;
uint32_t WINDOW_HEIGHT = 1080;

int main() {
    // Create platform.
#ifdef PATHFINDER_USE_VULKAN
    auto platform = PlatformVk::create(WINDOW_WIDTH, WINDOW_HEIGHT);
#else
    auto platform = PlatformGl::create(WINDOW_WIDTH, WINDOW_HEIGHT);
#endif

    // Create driver via platform.
    auto driver = platform->create_driver();

    // Create swap chain via platform.
    auto swap_chain = platform->create_swap_chain(driver, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create app.
    auto area_lut_input = load_file_as_bytes(PATHFINDER_ASSET_DIR"area-lut.png");
    auto svg_input = load_file_as_string(PATHFINDER_ASSET_DIR"tiger.svg");

    App app(driver, swap_chain, WINDOW_WIDTH, WINDOW_HEIGHT, area_lut_input, svg_input);

    // Main loop.
    while (!glfwWindowShouldClose(platform->get_glfw_window())) {
        platform->poll_events();

        // Acquire next image.
        if (!swap_chain->acquire_image()) continue;

        app.loop(swap_chain);

        swap_chain->flush();
    }

    app.cleanup();

    swap_chain->cleanup();

    platform->cleanup();

    return 0;
}
