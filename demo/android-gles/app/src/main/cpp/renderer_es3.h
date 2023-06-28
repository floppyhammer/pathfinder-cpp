#ifndef ANDROID_RENDERER_ES3_H
#define ANDROID_RENDERER_ES3_H

#include "gles3_jni.h"
#include "app.h"

#include <EGL/egl.h>
#include <android/asset_manager.h>

class RendererES3 : public RendererES {
public:
    explicit RendererES3(AAssetManager *_asset_manager);

    ~RendererES3() override;

    void init(int width, int height);

    void render() override;

private:
    EGLContext egl_context;

    Vec2I window_size;

    std::shared_ptr <Pathfinder::Device> device;
    std::shared_ptr <App> app;
    std::shared_ptr <TextureRect> texture_rect;
};

#endif //ANDROID_RENDERER_ES3_H
