#pragma once

#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <app.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <pathfinder/gpu/swap_chain.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "threaded_app", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "threaded_app", __VA_ARGS__))
#define LOGW_ONCE(...)                                         \
    do {                                                       \
        static bool alogw_once##__FILE__##__LINE__##__ = true; \
        if (alogw_once##__FILE__##__LINE__##__) {              \
            alogw_once##__FILE__##__LINE__##__ = false;        \
            LOGW(__VA_ARGS__);                                 \
        }                                                      \
    } while (0)

class NativeEngine {
public:
    explicit NativeEngine(struct android_app *app) {
        mAppCtx = app;
    }

    virtual ~NativeEngine() = default;

    virtual bool init_app() = 0;

    virtual void draw_frame();

    bool is_ready() const;

    virtual void set_context() {};

protected:
    void init_app_common(Pathfinder::Vec2I window_size);

    android_app *mAppCtx;

    std::shared_ptr<App> pf_app;
    std::shared_ptr<Blit> pf_blit;

    std::shared_ptr<Pathfinder::WindowBuilder> window_builder;
    std::shared_ptr<Pathfinder::Window> pf_window;
    std::shared_ptr<Pathfinder::Device> pf_device;
    std::shared_ptr<Pathfinder::Queue> pf_queue;
    std::shared_ptr<Pathfinder::SwapChain> pf_swapchain;
};
