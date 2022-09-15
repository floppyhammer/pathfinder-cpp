#include "../common/app.h"

#include "../src/gpu/platform.h"

uint32_t WINDOW_WIDTH = 1280;
uint32_t WINDOW_HEIGHT = 720;

int main() {
#ifdef PATHFINDER_USE_VULKAN
    auto device_type = Pathfinder::DeviceType::Vulkan;
#else
    auto device_type = Pathfinder::DeviceType::OpenGl4;
#endif

    // Create platform.
    auto platform = Pathfinder::Platform::new_impl(device_type, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create driver via platform.
    auto driver = platform->create_driver();

    // Create swap chain via platform.
    auto swap_chain = platform->create_swap_chain(driver, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create app.
    auto area_lut_input = Pathfinder::load_file_as_bytes(PATHFINDER_ASSET_DIR"area-lut.png");
    auto svg_input = Pathfinder::load_file_as_bytes(PATHFINDER_ASSET_DIR"tiger.svg");

    App app(driver, swap_chain, WINDOW_WIDTH, WINDOW_HEIGHT, area_lut_input, svg_input);

    // Main loop.
    while (!glfwWindowShouldClose(platform->get_glfw_window())) {
        platform->poll_events();

        // Acquire next image.
        if (!swap_chain->acquire_image()) continue;

        app.loop(swap_chain);

        swap_chain->flush();
    }

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    app.cleanup();

    platform->cleanup();

    return 0;
}
