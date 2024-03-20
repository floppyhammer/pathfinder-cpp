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

    Pathfinder::Vec2I window_size;

    std::shared_ptr<App> pf_app;
    std::shared_ptr<Blit> pf_blit;

    std::shared_ptr<Pathfinder::WindowBuilder> pf_window_builder;
    std::shared_ptr<Pathfinder::Window> pf_window;
    std::shared_ptr<Pathfinder::Device> pf_device;
    std::shared_ptr<Pathfinder::Queue> pf_queue;
    std::shared_ptr<Pathfinder::SwapChain> pf_swapchain;
};

#endif //ANDROID_RENDERER_ES3_H
