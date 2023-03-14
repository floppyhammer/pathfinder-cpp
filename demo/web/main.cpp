#include "../common/app.h"
#include "../src/gpu/window.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

using namespace Pathfinder;

const int32_t WINDOW_WIDTH = 640;
const int32_t WINDOW_HEIGHT = 480;

App* app;
TextureRect* texture_rect;

void render(void* _swap_chain) {
    app->update();

    auto* swap_chain = static_cast<SwapChain*>(_swap_chain);

    auto cmd_buffer = swap_chain->get_command_buffer();

    auto framebuffer = swap_chain->get_framebuffer();

    // Swap chain render pass.
    {
        cmd_buffer->begin_render_pass(swap_chain->get_render_pass(),
                                      framebuffer,
                                      Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        // Draw canvas to screen.
        texture_rect->draw(cmd_buffer, framebuffer->get_size());

        cmd_buffer->end_render_pass();
    }

    cmd_buffer->submit();

    swap_chain->flush();
}

int main() {
    Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

    auto window = Window::new_impl(DeviceType::WebGl2, window_size);

    auto driver = window->create_driver();

    auto swap_chain = window->create_swap_chain(driver);

    // Create app.
    app = new App(driver, window_size, {}, {});

    auto dst_texture = driver->create_texture({window_size, TextureFormat::Rgba8Unorm, "dst texture"});

    app->canvas->set_dst_texture(dst_texture);

    // Set viewport texture to a texture rect.
    texture_rect = new TextureRect(driver, swap_chain->get_render_pass(), window_size.to_f32());
    texture_rect->set_texture(dst_texture);

    emscripten_set_main_loop_arg(render, swap_chain.get(), 0, 1);

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    delete app;
    delete texture_rect;

    window->cleanup();

    return 0;
}
