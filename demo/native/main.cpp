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

    // Create driver.
    auto driver = platform->create_driver();

    // Create swap chain.
    //auto swap_chain = driver->create_swap_chain(WINDOW_WIDTH, WINDOW_HEIGHT, platform);
    auto swap_chain = std::make_shared<SwapChainVk>(WINDOW_WIDTH, WINDOW_HEIGHT, platform, driver);

    // Start input server.
    InputServer::get_singleton();

    // Create app.
    auto area_lut_input = load_file_as_bytes(PATHFINDER_ASSET_DIR"area-lut.png");
    auto font_input = load_file_as_bytes(PATHFINDER_ASSET_DIR"OpenSans-Regular.ttf");
    auto svg_input = load_file_as_string(PATHFINDER_ASSET_DIR"tiger.svg");

    App app(driver, WINDOW_WIDTH, WINDOW_HEIGHT, area_lut_input, font_input, svg_input);

    // Main loop.
    while (!glfwWindowShouldClose(platform->get_glfw_window())) {
        platform->handle_inputs();

        app.loop(swap_chain);

        platform->swap_buffers_and_poll_events();
    }

    platform->cleanup();

    return 0;
}
