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

class RendererES2: public Renderer {
public:
    RendererES2(AAssetManager *pAssetManager);
    ~RendererES2() override;
    bool init(int width, int height);

private:
    void draw() override;

    const EGLContext mEglContext;
};

Renderer* createES2Renderer(int width, int height, AAssetManager *p_asset_manager) {
    auto* renderer = new RendererES2(p_asset_manager);
    if (!renderer->init(width, height)) {
        delete renderer;
        return nullptr;
    }
    return renderer;
}

RendererES2::RendererES2(AAssetManager *p_asset_manager)
        : Renderer(p_asset_manager), mEglContext(eglGetCurrentContext()) {}

bool RendererES2::init(int width, int height) {
    ALOGV("Using OpenGL ES 2.0 renderer");
    return true;
}

RendererES2::~RendererES2() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
}

void RendererES2::draw() {
}
