/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gles3jni.h"
#include <EGL/egl.h>
#include <android/asset_manager.h>

#include "demo/common/app.h"

#include <iostream>
#include <chrono>

class RendererES3 : public Renderer {
public:
    explicit RendererES3(AAssetManager *p_asset_manager);

    ~RendererES3() override;

    void init(int width, int height);

    void render() override;

private:
    EGLContext mEglContext;

    std::shared_ptr<App> app;
    std::shared_ptr<Pathfinder::Driver> driver;
    std::shared_ptr<TextureRect> texture_rect;

    int width;
    int height;
};

Renderer *createES3Renderer(int width, int height, AAssetManager *p_asset_manager) {
    auto *renderer = new RendererES3(p_asset_manager);
    renderer->init(width, height);

    return renderer;
}

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

RendererES3::RendererES3(AAssetManager *p_asset_manager)
        : Renderer(p_asset_manager), mEglContext(eglGetCurrentContext()) {}

void RendererES3::init(int width, int height) {
    ALOGV("Using OpenGL ES 3.0 renderer");

    auto svg_input = get_asset_file(asset_manager, "tiger.svg");

    driver = std::make_shared<Pathfinder::DriverGl>();

    app = std::make_shared<App>(driver, width, height, svg_input);

    // Set viewport texture to a texture rect.
    texture_rect = std::make_shared<TextureRect>(driver,
                                                 nullptr,
                                                 width,
                                                 height);
    texture_rect->set_texture(app->canvas->get_dest_texture());
}

RendererES3::~RendererES3() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext) {
        return;
    }
}

void RendererES3::render() {
    app->update();

    auto cmd_buffer = driver->create_command_buffer(true);

    texture_rect->draw(driver, cmd_buffer,
                       {(uint32_t) texture_rect->size.x,
                        (uint32_t) texture_rect->size.y});

    cmd_buffer->submit(driver);
}
