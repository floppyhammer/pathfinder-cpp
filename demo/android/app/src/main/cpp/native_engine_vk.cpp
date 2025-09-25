#include "native_engine_vk.h"

#include <android/log.h>
#include <pathfinder/gpu/vk/window_builder.h>

#include <cassert>
#include <cstring>
#include <vector>

#include "app.h"
#include "vulkan_wrapper.h"

NativeEngineVk::NativeEngineVk(struct android_app *app) : NativeEngine(app) {}

bool NativeEngineVk::init_app() {
    if (!InitVulkan()) {
        Pathfinder::Logger::warn("Vulkan is unavailable, install vulkan and re-start");
        return false;
    }

    auto window_size =
        Pathfinder::Vec2I(ANativeWindow_getWidth(mAppCtx->window), ANativeWindow_getHeight(mAppCtx->window));

    window_builder = std::make_shared<Pathfinder::WindowBuilderVk>(mAppCtx->window, window_size);

    init_app_common(window_size);

    return true;
}
