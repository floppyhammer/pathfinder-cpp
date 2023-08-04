#include "renderer_es3.h"

#include <iostream>
#include <chrono>

std::vector<char> get_asset_file(AAssetManager *asset_manager, const char *p_filename) {
    AAssetDir *asset_dir = AAssetManager_openDir(asset_manager, "");

    std::vector<char> buffer;
    const char *filename;

    while ((filename = AAssetDir_getNextFileName(asset_dir)) != nullptr) {
        ALOGV("Found asset file: %s", filename);

        // Search for desired file.
        if (!strcmp(filename, p_filename)) {
            ALOGV("Read asset file: %s", filename);

            AAsset *asset = AAssetManager_open(asset_manager, filename, AASSET_MODE_STREAMING);

            //holds size of searched file
            off64_t length = AAsset_getLength64(asset);
            //keeps track of remaining bytes to read
            off64_t remaining = AAsset_getRemainingLength64(asset);
            size_t Mb = 1000 * 1024; // 1Mb is maximum chunk size for compressed assets
            size_t curr_chunk;
            buffer.reserve(length);

            //while we have still some data to read
            while (remaining != 0) {
                //set proper size for our next chunk
                if (remaining >= Mb) {
                    curr_chunk = Mb;
                } else {
                    curr_chunk = remaining;
                }
                char chunk[curr_chunk];

                //read data chunk
                // returns less than 0 on error
                if (AAsset_read(asset, chunk, curr_chunk) > 0) {
                    //and append it to our vector
                    buffer.insert(buffer.end(), chunk, chunk + curr_chunk);
                    remaining = AAsset_getRemainingLength64(asset);
                }
            }
            AAsset_close(asset);
        }
    }

    return buffer;
}

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

    auto svg_input = get_asset_file(asset_manager, "features.svg");
    auto img_input = get_asset_file(asset_manager, "sea.png");

    // Wrap a device.
    device = std::make_shared<Pathfinder::DeviceGl>();

    queue = std::make_shared<Pathfinder::QueueGl>();

    app = std::make_shared<App>(device, queue, svg_input, img_input);

    auto dst_texture = device->create_texture(
            {window_size, TextureFormat::Rgba8Unorm}, "dst texture");

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
