#include "renderer_es3.h"

#include <iostream>
#include <chrono>

RendererES *createES3Renderer(int width, int height, AAssetManager *_asset_manager) {
    auto *renderer = new RendererES3(_asset_manager);
    renderer->init(width, height);

    return renderer;
}

RendererES3::RendererES3(AAssetManager *_asset_manager)
        : RendererES(_asset_manager), egl_context(eglGetCurrentContext()) {}

void RendererES3::init(int width, int height) {
    ALOGV("Using OpenGL ES 3.0 renderer");

    window_size = {width, height};

    auto svg_input = Pathfinder::load_asset(asset_manager, "features.svg");
    auto img_input = Pathfinder::load_asset(asset_manager, "sea.png");

    // Wrap a device.
    device = std::make_shared<Pathfinder::DeviceGl>();

    queue = std::make_shared<Pathfinder::QueueGl>();

    app = std::make_shared<App>(device, queue, window_size, svg_input, img_input);

    auto dst_texture = device->create_texture(
            {window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

    app->canvas->set_dst_texture(dst_texture);

    texture_rect = std::make_shared<TextureRect>(device, queue, nullptr);
    texture_rect->set_texture(dst_texture);
}

RendererES3::~RendererES3() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != egl_context) {
        return;
    }
}

void RendererES3::render() {
    app->update();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_size.x, window_size.y);

    auto encoder = device->create_command_encoder("");

    texture_rect->draw(encoder, window_size);

    queue->submit_and_wait(encoder);
}
